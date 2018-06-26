// https://github.com/KubaO/stackoverflown/tree/master/questions/winreg-51027141
#include <QtCore>
#include <windows.h>

static LPCSTR toWChar(const QString &str) {
  return reinterpret_cast<const wchar_t*>(str.utf16());
}
static LPSTR toWChar(QString &str) {
  return const_cast<wchar_t*>(toWChar(const_cast<const QString &>(str)));
}

class RegKey {
  HKEY handle = INVALID_HANDLE_VALUE;
  LONG numeric() const { return (LONG)(ULONG_PTR)handle; }
  explicit RegKey(HKEY handle) : handle(handle) {}
public:
  bool isValid() const { return handle != INVALID_HANDLE_VALUE; }
  bool isPredefined() const {
    return numeric() >= (LONG)HKEY_CLASSES_ROOT
           && numeric() <= (LONG)HKEY_PERFORMANCE_NLSTEXT;
  }
  bool isOwned() const { return numeric() >= 0; }
  static RegKey HKLM() { return RegKey(HKEY_LOCAL_MACHINE); }
  explicit RegKey(const RegKey &key, const QString &subKey) {
    RegOpenKeyEx(key.handle, toWChar(subKey), 0, KEY_ALL_ACCESS, &handle);
  }
  RegKey(RegKey &&o) { swap(o); }
  RegKey &operator=(RegKey &&o) { return swap(o), *this; }
  RegKey(const RegKey &) = delete;
  RegKey &operator=(const RegKey &) = delete;

  void swap(RegKey &o) {
    using std::swap;
    swap(handle, o.handle);
  }
  ~RegKey() {
    if (isOwned())
      RegCloseKey(handle);
  }
  QString getString(const QString &subKey, const QString &name) const {
    DWORD length = 0;
    if (RegGetValue(handle, toWChar(subKey), toWChar(name), RRF_RT_REG_SZ,
        NULL, NULL, &length) != ERROR_SUCCESS)
      return {};
    QString ret;
    ret.resize(length);
    if (RegGetValue(handle, toWChar(subKey), toWChar(name), RRF_RT_REG_SZ,
        NULL, toWChar(ret), &length) != ERROR_SUCCESS)
      return {};
    return ret;
  }
};

void swap(RegKey &lhs, RegKey &rhs) {
  lhs.swap(rhs);
}

int main(int argc, char *argv[]) {

}
