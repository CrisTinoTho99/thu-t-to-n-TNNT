# GLAD (vendored)

Loader OpenGL 3.3 core sinh sẵn, không cần Python khi build.

- Generator: **glad 2.0.8** (`glad2` trên PyPI, https://github.com/Dav1dde/glad)
- Lệnh sinh:

```bash
glad --reproducible --api gl:core=3.3 --extensions "" --out-path external/glad c --loader
```

- API: `gl:core=3.3`; extension: không có; loader nội bộ: bật; chế độ reproducible.
- File sinh ra: `include/glad/gl.h`, `include/KHR/khrplatform.h`, `src/gl.c`.
- Giấy phép của mã sinh: WTFPL hoặc CC0-1.0, cộng Apache-2.0 (xem header từng file).

Không sửa tay các file trong thư mục này. Khi cần nâng cấp, chạy lại đúng lệnh trên với phiên bản glad mới rồi build/test lại.
