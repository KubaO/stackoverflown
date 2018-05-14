// https://github.com/KubaO/stackoverflown/tree/master/questions/variant-take
#include <QtGui>
#include <string>
#include <type_traits>

namespace detail {
template <class C> class is_qt_shared {
  template <class T> static std::true_type testDetach(void (T::*)());
  template <class T> static std::true_type testIsDetached(bool (T::*)() const);

  template <class T> static decltype(testDetach(&T::detach)) test1(std::nullptr_t);
  template <class T> static std::false_type test1(...);

  template <class T> static decltype(testIsDetached(&T::isDetached)) test2(std::nullptr_t);
  template <class T> static std::false_type test2(...);
public:
  static constexpr bool value = decltype(test1<C>(nullptr))::value && decltype(test2<C>(nullptr))::value;
};

template <typename T> std::enable_if_t<is_qt_shared<T>::value, bool> isDetached(const T& val) {
  return val.isDetached();
}
template <typename T> std::enable_if_t<!is_qt_shared<T>::value, bool> isDetached(const T&) {
  return true;
}

template <class D> class has_moving_setValue {
  using data_type = std::decay_t<D>;
  template <class T> static std::true_type testSetValue(void (T::*)(D&&));

  template <class T> static decltype(testSetValue(&T::template setValue<data_type>)) test(std::nullptr_t);
  template <class T> static std::false_type test(...);
public:
  static constexpr bool value = decltype(test<QVariant>(nullptr))::value;
};

template <class C> class has_clear {
  template <class T> static std::true_type testClear(void (T::*)());

  template <class T> static decltype(testClear(&T::clear)) test(std::nullptr_t);
  template <class T> static std::false_type test(...);
public:
  static constexpr bool value = decltype(test<C>(nullptr))::value;
};

template <typename T> std::enable_if_t<has_clear<T>::value> clear(T &val) { val.clear(); }
template <typename T> std::enable_if_t<!has_clear<T>::value> clear(T &val) { val = {}; }

template <typename T> bool isValid(const QVariant &var) {
  return var.isValid() && static_cast<int>(var.userType()) == qMetaTypeId<std::decay_t<T>>();
}

template <typename T> T getValue(QVariant &var) {
  Q_ASSERT(var.userType() == qMetaTypeId<std::decay_t<T>>());
  return std::move(*reinterpret_cast<T*>(var.data()));
}

} // detail

template <typename T> bool canUseValue(const QVariant &src) {
  return detail::isValid<T>(src);
}

template <typename T> T &useValue(QVariant &src) {
  if (detail::isValid<T>(src))
    return *reinterpret_cast<T*>(src.data());
  static T s;
  return s;
}

template <typename T, typename ...Args> T takeValue(QVariant &src, Args ...args) {
  if (detail::isValid<T>(src))
    return detail::getValue<T>(src);
  return T{std::forward<Args>(args)...};
}

template <typename T, typename ...Args> T reuseValue(QVariant &src, Args ...args) {
  if (detail::isValid<T>(src) && src.isDetached()) {
    auto var = std::move(src);
    Q_ASSERT(var.isDetached());
    auto const val = detail::getValue<T>(var);
    if (detail::isDetached(val))
      return val;
  }
  return T{std::forward<Args>(args)...};
}

template <typename T> void setValue(QVariant &dst, T &&val) {
  if (detail::isValid<T>(dst))
    *reinterpret_cast<T*>(dst.data()) = std::forward<T>(val);
  else {
    dst.setValue(std::forward<T>(val));
    if (std::is_rvalue_reference<decltype(val)>::value)
      if (!detail::has_moving_setValue<std::decay_t<T>>::value)
        detail::clear(val);
  }
}

template <typename T> bool canUseProperty(const QObject *obj, const char *name) {
  int id = obj->metaObject()->indexOfProperty(name);
  if (id >= 0)
    return false; // it's not a dynamic property
  auto const prop = obj->property(name);
  return detail::isValid<T>(prop) && prop.data_ptr().is_shared && prop.data_ptr().data.shared->ref.load() == 2;
}

template <typename T> T &useProperty(QObject *obj, const char *name) {
  if (canUseProperty<T>(obj, name))
    return *const_cast<T*>(reinterpret_cast<const T*>(obj->property(name).constData()));
  static T s;
  return s;
}

template <typename T, typename ...Args> T takeProperty(QObject *obj, const char *name, Args ...args) {
  auto prop = obj->property(name);
  if (detail::isValid<T>(prop)) {
    obj->setProperty(name, {});
    auto const val = detail::getValue<T>(prop);
    if (prop.data_ptr().is_shared) // reuse the same storage in QVariant
      obj->setProperty(name, std::move(prop));
    return val;
  }
  return T{std::forward<Args>(args)...};
}

template <typename T, typename ...Args> T reuseProperty(QObject *obj, const char *name, Args ...args) {
  auto prop = obj->property(name);
  if (detail::isValid<T>(prop)) {
    obj->setProperty(name, {});
    if (prop.isDetached()) {
      auto const val = detail::getValue<T>(prop);
      if (detail::isDetached(val)) {
        if (prop.data_ptr().is_shared) // reuse the same storage in QVariant
          obj->setProperty(name, prop);
        return val;
      }
    }
  }
  return T{std::forward<Args>(args)...};
}

template <typename T> void setProperty(QObject *obj, const char *name, T &&val) {
  auto prop = obj->property(name);
  obj->setProperty(name, {});
  setValue(prop, std::forward<T>(val));
  obj->setProperty(name, std::move(prop));
}

//

template <typename T> struct qvariant_is_shared {
  static constexpr bool value =
      sizeof(T) > sizeof(QVariant::Private::Data) || (!QTypeInfoQuery<T>::isRelocatable && !std::is_enum<T>::value);
};

bool is_short(const std::string &str) {
  auto *start = (const char *)&str;
  return str.data() >= start && str.data() < start + sizeof(str);
}

Q_DECLARE_METATYPE(std::string)

int main() {
  const char kProp[] = "prop";
  QObject obj;

  Q_STATIC_ASSERT(qvariant_is_shared<std::string>::value == true);
  std::string const val(128, 'a');
  Q_ASSERT(!is_short(val));
  obj.setProperty(kProp, QVariant::fromValue(val));
  auto ss = takeProperty<std::string>(&obj, kProp);
  Q_ASSERT(ss == val);
  Q_ASSERT(obj.property(kProp).value<std::string>().empty());
  auto *ssData = ss.data();
  setProperty(&obj, kProp, std::move(ss));
  Q_ASSERT(ss.empty());
  ss = reuseProperty<std::string>(&obj, kProp);
  Q_ASSERT(ss == val);
  Q_ASSERT(obj.property(kProp).value<std::string>().empty());
  Q_ASSERT(ss.data() == ssData);
  setProperty(&obj, kProp, std::move(ss));
  Q_ASSERT(ss.empty());
  ss = reuseProperty<std::string>(&obj, kProp);
  Q_ASSERT(ss == val);
  Q_ASSERT(obj.property(kProp).value<std::string>().empty());
  Q_ASSERT(ss.data() == ssData);
  setProperty(&obj, kProp, std::move(ss));
  Q_ASSERT(ss.empty());
  Q_ASSERT(canUseProperty<std::string>(&obj, kProp));
  auto &ssr = useProperty<std::string>(&obj, kProp);
  Q_ASSERT(ssr.data() == ssData);

  Q_STATIC_ASSERT(qvariant_is_shared<QString>::value == false);
  obj.setProperty(kProp, QString("Foo"));
  auto sv = obj.property(kProp);
  auto *sData = sv.value<QString>().constData();
  Q_ASSERT(!sv.value<QString>().isDetached());
  sv.clear();
  auto s = reuseProperty<QString>(&obj, kProp);
  Q_ASSERT(s.isDetached() && s == "Foo");
  Q_ASSERT(s.constData() == sData);
  Q_ASSERT(!canUseProperty<QString>(&obj, kProp)); // it's not shared

  Q_STATIC_ASSERT(qvariant_is_shared<QImage>::value == true);
  obj.setProperty(kProp, QImage{1, 1, QImage::Format_ARGB32});
  auto i = takeProperty<QImage>(&obj, kProp);
  Q_ASSERT(obj.property(kProp).value<QImage>().isNull());
  auto *ivData = obj.property(kProp).constData();
  auto *iData = i.constBits();
  setProperty(&obj, kProp, std::move(i));
  Q_ASSERT(i.isNull());
  i = reuseProperty<QImage>(&obj, kProp);
  Q_ASSERT(i.constBits() == iData);
  setProperty(&obj, kProp, std::move(i));
  Q_ASSERT(i.isNull());
  Q_ASSERT(obj.property(kProp).constData() == ivData);
  Q_ASSERT(canUseProperty<QImage>(&obj, kProp));
  auto const &ir = useProperty<QImage>(&obj, kProp);
  Q_ASSERT(ir.constBits() == iData);
}
