// https://github.com/KubaO/stackoverflown/tree/master/questions/implicit-list-44593216
#include <QtCore>
#include <type_traits>

//

class PolymorphicSharedData : public QSharedData {
public:
   virtual PolymorphicSharedData * clone() const = 0;
   virtual QDebug dump(QDebug) const = 0;
   virtual ~PolymorphicSharedData() {}
};

class PolymorphicShared {
protected:
   QSharedDataPointer<PolymorphicSharedData> d_ptr;
   PolymorphicShared(PolymorphicSharedData * d) : d_ptr(d) {}
public:
   PolymorphicShared() = default;
   PolymorphicShared(const PolymorphicShared & o) = default;
   PolymorphicShared & operator=(const PolymorphicShared &) = default;
   QDebug dump(QDebug dbg) const { return d_ptr->dump(dbg); }
   template <class T> typename
   std::enable_if<std::is_pointer<T>::value, typename
   std::enable_if<!std::is_const<typename std::remove_pointer<T>::type>::value, T>::type>
   ::type as() {
      if (dynamic_cast<typename std::remove_pointer<T>::type::PIMPL*>(d_ptr.data()))
         return static_cast<T>(this);
      return {};
   }
   template <class T> typename
   std::enable_if<std::is_pointer<T>::value, typename
   std::enable_if<std::is_const<typename std::remove_pointer<T>::type>::value, T>::type>
   ::type as() const {
      if (dynamic_cast<const typename std::remove_pointer<T>::type::PIMPL*>(d_ptr.data()))
         return static_cast<T>(this);
      return {};
   }
   template <class T> typename
   std::enable_if<std::is_reference<T>::value, typename
   std::enable_if<!std::is_const<typename std::remove_reference<T>::type>::value, T>::type>
   ::type as() {
      Q_UNUSED(dynamic_cast<typename std::remove_reference<T>::type::PIMPL&>(*d_ptr));
      return static_cast<T>(*this);
   }
   template <class T> typename
   std::enable_if<std::is_reference<T>::value, typename
   std::enable_if<std::is_const<typename std::remove_reference<T>::type>::value, T>::type>
   ::type as() const {
      Q_UNUSED(dynamic_cast<const typename std::remove_reference<T>::type::PIMPL&>(*d_ptr));
      return static_cast<T>(*this);
   }
   int ref() const { return d_ptr ? d_ptr->ref.load() : 0; }
};

QDebug operator<<(QDebug dbg, const PolymorphicShared & val) {
   return val.dump(dbg);
}

Q_DECLARE_TYPEINFO(PolymorphicShared, Q_MOVABLE_TYPE);

#define DECLARE_TYPEINFO(concreteType) Q_DECLARE_TYPEINFO(concreteType, Q_MOVABLE_TYPE)

template <> PolymorphicSharedData * QSharedDataPointer<PolymorphicSharedData>::clone() {
   return d->clone();
}

//

template <class Data, class Base = PolymorphicShared> class PolymorphicSharedBase : public Base {
   friend class PolymorphicShared;
protected:
   using PIMPL = typename std::enable_if<std::is_base_of<PolymorphicSharedData, Data>::value, Data>::type;
   PIMPL * d() { return static_cast<PIMPL*>(&*this->d_ptr); }
   const PIMPL * d() const { return static_cast<const PIMPL*>(&*this->d_ptr); }
   PolymorphicSharedBase(PolymorphicSharedData * d) : Base(d) {}
   template <typename T> static typename std::enable_if<std::is_constructible<T>::value, T*>::type
   construct() { return new T(); }
   template <typename T> static typename std::enable_if<!std::is_constructible<T>::value, T*>::type
   construct() { return nullptr; }
public:
   using Base::Base;
   template<typename ...Args,
            typename = typename std::enable_if<std::is_constructible<Data, Args...>::value>::type
            > PolymorphicSharedBase(Args&&... args) :
      Base(static_cast<PolymorphicSharedData*>(new Data(std::forward<Args>(args)...))) {}
   PolymorphicSharedBase() : Base(construct<Data>()) {}
};

///

class MyAbstractTypeData : public PolymorphicSharedData {
public:
   virtual void gurgle() = 0;
   virtual int gargle() const = 0;
};

class MyAbstractType : public PolymorphicSharedBase<MyAbstractTypeData> {
public:
   using PolymorphicSharedBase::PolymorphicSharedBase;
   void gurgle() { d()->gurgle(); }
   int gargle() const { return d()->gargle(); }
};
DECLARE_TYPEINFO(MyAbstractType);

//

class FooTypeData : public MyAbstractTypeData {
protected:
   int m_foo = 0;
public:
   FooTypeData() = default;
   FooTypeData(int data) : m_foo(data) {}
   void gurgle() override { m_foo++; }
   int gargle() const override { return m_foo; }
   MyAbstractTypeData * clone() const override { return new FooTypeData(*this); }
   QDebug dump(QDebug dbg) const override {
      return dbg << "FooType-" << ref << ":" << m_foo;
   }
};

using FooType = PolymorphicSharedBase<FooTypeData, MyAbstractType>;
DECLARE_TYPEINFO(FooType);

//

class BarTypeData : public FooTypeData {
protected:
   int m_bar = 0;
public:
   BarTypeData() = default;
   BarTypeData(int data) : m_bar(data) {}
   MyAbstractTypeData * clone() const override { return new BarTypeData(*this); }
   QDebug dump(QDebug dbg) const override {
      return dbg << "BarType-" << ref << ":" << m_foo << "," << m_bar;
   }
   virtual void murgle() { m_bar++; }
};

class BarType : public PolymorphicSharedBase<BarTypeData, FooType> {
public:
   using PolymorphicSharedBase::PolymorphicSharedBase;
   void murgle() { d()->murgle(); }
};
DECLARE_TYPEINFO(BarType);

//

template <typename F> bool is_bad_cast(F && fun) {
   try { fun(); } catch (std::bad_cast) { return true; }
   return false;
}

int main() {
   Q_ASSERT(sizeof(FooType) == sizeof(void*));
   MyAbstractType a;
   Q_ASSERT(!a.as<FooType*>());
   FooType foo;
   Q_ASSERT(foo.as<FooType*>());
   a = foo;
   Q_ASSERT(a.ref() == 2);
   Q_ASSERT(a.as<const FooType*>());
   Q_ASSERT(a.ref() == 2);
   Q_ASSERT(a.as<FooType*>());
   Q_ASSERT(a.ref() == 1);
   MyAbstractType a2(foo);
   Q_ASSERT(a2.ref() == 2);

   QList<MyAbstractType> list1{FooType(3), BarType(8)};
   auto list2 = list1;
   qDebug() << "After copy:         " << list1 << list2;
   list2.detach();
   qDebug() << "After detach:       " << list1 << list2;
   list1[0].gurgle();
   qDebug() << "After list1[0] mod: " << list1 << list2;

   Q_ASSERT(list2[1].as<BarType*>());
   list2[1].as<BarType&>().murgle();
   qDebug() << "After list2[1] mod: " << list1 << list2;

   Q_ASSERT(!list2[0].as<BarType*>());
   Q_ASSERT(is_bad_cast([&]{ list2[0].as<BarType&>(); }));

   auto const list3 = list1;
   Q_ASSERT(!list3[0].as<const BarType*>());
   Q_ASSERT(is_bad_cast([&]{ list3[0].as<const BarType&>(); }));
}
