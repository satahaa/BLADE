# 🎯 BLADE - Simple HTTP Server

## ✅ Simplified! No More SSL/HTTPS

All SSL/HTTPS complexity has been removed. Pure HTTP now!

---

## 🚀 Quick Start

```bash
cd D:\Study\3-1\Project\BLADE\cmake-build-debug
blade.exe
```

**Access:**
```
http://localhost           (your PC)
http://192.168.x.x        (other devices)
```

---

## 📊 What Changed

| Aspect | Before | After |
|--------|--------|-------|
| **Protocol** | HTTPS | HTTP |
| **Port** | 443 | 80 |
| **URL** | https://ip:443 | http://ip |
| **Security Warnings** | ❌ Yes | ✅ No |
| **Certificate Needed** | ❌ Yes | ✅ No |
| **Admin Required** | ❌ Often | ✅ Rarely |
| **Code Lines** | 461 | 245 |

---

## 🎉 Benefits

- ✅ **No browser security warnings**
- ✅ **No certificate errors**
- ✅ **Direct, instant access**
- ✅ **47% less code**
- ✅ **Simpler to maintain**
- ✅ **Faster development**

---

## 🌐 Access Examples

### No Auth Mode
```bash
blade.exe
```
Browser: `http://192.168.1.100` → Dashboard loads immediately

### With Auth
```bash
blade.exe -u admin -p pass123
```
Browser: `http://192.168.1.100` → Login page → Dashboard

---

## 📝 Help

```bash
blade.exe -h
```

---

## 🔧 Technical Details

### Removed
- ~216 lines of SSL code
- Windows certificate store operations
- Schannel credential handling
- Self-signed certificate creation
- All HTTPS complexity

### Changed
- Port: 443 → 80
- Protocol: HTTPS → HTTP
- URLs: https:// → http://

---

## ✅ Perfect for Local Networks!

**Simple. Fast. No nonsense. Just HTTP!** 🚀

