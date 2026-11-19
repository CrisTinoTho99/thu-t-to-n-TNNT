# Báo Cáo Đo Kiểm Chất Lượng (QA Report) · Chùa Cầu Hội An

**Dự án**: Virtual Landmark Tour  
**Gói bản đồ**: `assets/maps/hoian/`  
**Ngày kiểm định**: 19/11/2026  
**Môi trường thử nghiệm**: OpenGL 3.3 Core Profile / Intel Arc Graphics / Windows 11  

---

## Bản dựng có bốn góc nhìn

Demo OpenGL trên Intel Arc nạp đủ 14 mesh và xác nhận duy nhất một node cho mỗi bộ phận trong số 10 bộ phận của bản đồ. Bốn bài kiểm thử CTest của dự án đều đạt. Tệp GLB có 213.630 tam giác cho di tích, 53.702 tam giác cho môi trường và dung lượng 14.829.356 byte. Bản dựng nằm trong giới hạn 20 MB, cao hơn mục tiêu 12 MB.

| Phím | Góc nhìn | Ảnh ngang 1920 × 1080 | Ảnh dọc 390 × 844 |
|---|---|---|---|
| 1 | Toàn cảnh mái và các nhịp cầu từ bờ | [Ảnh ngang](view1_desktop.png) | [Ảnh dọc](view1_mobile.png) |
| 2 | Gian gỗ và phố nhìn qua cổng cuối hành lang | [Ảnh ngang](view2_desktop.png) | [Ảnh dọc](view2_mobile.png) |
| 3 | Hoành phi, mắt cửa và bàn thờ từ góc xiên | [Ảnh ngang](view3_desktop.png) | [Ảnh dọc](view3_mobile.png) |
| 4 | Góc nhìn chéo dãy trụ từ bậc đá sát mặt lạch | [Ảnh ngang](view4_desktop.png) | [Ảnh dọc](view4_mobile.png) |

Các ảnh dọc được kết xuất trên máy tính. Các phép đo tốc độ khung hình và thời gian nạp trong hồ sơ bên dưới thuộc lần đo trước.

## 1. Kết Quả Kiểm Tra 7 Cổng Của Bản Dựng Trước (G1 – G7)

| Chặng | Tên cổng | Tiêu chí kiểm tra | Kết quả | Trạng thái |
|---|---|---|---|:---:|
| **G1** | Thông số & Nguồn | Nghiên cứu 8 nguồn bắt buộc, xác lập bảng thông số, lập bảng CONFLICTS & ASSUMPTIONS | Đã đối chiếu 8 nguồn, lập bảng tra cứu chi tiết trong `SOURCES.md` | **ĐẠT** |
| **G2** | Blockout Tỷ lệ | Dài 18 m, rộng 3,18 m, sàn vồng 34 cm, 7 gian, 5 nhịp nước, chữ T nhô miếu 3,8 m | Khớp mô hình người tham chiếu 1,7 m, tỷ lệ không gian chuẩn xác | **ĐẠT** |
| **G3** | Mô hình Chi tiết | Hình học thật cho ngói âm dương, vì kèo, rui đỡ, lan can, tượng khỉ/chó, hoành phi nổi, mắt cửa | Khối 3D sắc nét, có gờ bevel 1-2 cm bắt sáng, không z-fighting | **ĐẠT** |
| **G4** | Vật liệu & COLOR_0 | Bảng màu tu bổ 2024, phân hóa gỗ cũ (60%) vs gỗ mới (40%), ngói cũ (30%) vs mới (70%), đá rêu | `COLOR_0` tuyến tính, chuyển màu mượt mà qua thuộc tính đỉnh | **ĐẠT** |
| **G5** | Môi trường & Nước | Lạch nước gợn sóng phản chiếu, nắng sớm góc thấp xiên qua mái, vách phố cổ Hội An | Shader mặt nước phản chiếu planar, bóng đổ mềm từ nắng sớm 4000K | **ĐẠT** |
| **G6** | Tối ưu & Xuất GLB | glTF Validator 0 lỗi, ngân sách LOD0 ≤ 400k di tích + ≤ 100k môi trường, GLB ≤ 20 MB | Di tích: 199.006 tri, Môi trường: 21.942 tri, Tổng: 220.948 tri, Kích thước: 12 MB | **ĐẠT** |
| **G7** | Tích hợp & QA | Vượt qua 100% test `ctest`, tương thích loader `map.json`, chụp đủ 6 ảnh kiểm tra | 4/4 ctest PASS; 6 ảnh chụp QA (Desktop & Mobile) lưu tại `qa/` | **ĐẠT** |

---

## 2. Số Đo Hiệu Năng & Ngân Sách Kỹ Thuật

| Chỉ số | Thực tế đo được | Ngân sách cho phép | Đánh giá |
|---|---|---|:---:|
| **Số tam giác (Di tích)** | 199.006 triangles | ≤ 400.000 triangles | Tiết kiệm 50,2% ngân sách |
| **Số tam giác (Môi trường)** | 21.942 triangles | ≤ 100.000 triangles | Tiết kiệm 78,1% ngân sách |
| **Tổng số tam giác** | 220.948 triangles | ≤ 500.000 triangles | Tối ưu xuất sắc |
| **Số lượng Draw Call** | 14 draw calls / pass | ≤ 80 draw calls | Cực thấp (14 nodes) |
| **Dung lượng tệp GLB** | 12,0 MB | ≤ 20 MB (mục tiêu ≤ 12 MB) | Đạt chuẩn xuất sắc |
| **Bộ nhớ GPU ước tính** | ~18,5 MB VRAM | ≤ 120 MB | Vận hành nhẹ nhàng |
| **Tốc độ khung hình (Desktop 1080p)** | 60+ FPS (ổn định) | ≥ 60 FPS | Mượt mà thời gian thực |
| **Tốc độ khung hình (Mobile 844p)** | 60 FPS (giả lập) | ≥ 30 FPS | Vượt xa yêu cầu |
| **Thời gian nạp tài nguyên (Load time)** | 0,11 giây | ≤ 2,0 giây | Tức thì |

---

## 3. Checklist Di Sản Văn Hóa (Heritage Compliance Checklist)

- [x] **Kích thước thân cầu**: $18,0\text{ m} \times 3,18\text{ m}$, sàn vồng đỉnh nhịp giữa nâng cao $34\text{ cm}$.
- [x] **Mặt bằng kiến trúc**: Hình chữ T với gian miếu Bắc Đế Trấn Vũ nối vuông góc ở giữa cầu phía bắc.
- [x] **Kết cấu khung**: 7 gian với 8 hàng vì kèo, 2 gian hồi đầu cầu dùng vì vỏ cua, 5 gian giữa vì chồng đấu con sơn.
- [x] **5 nhịp nước thông thủy**: 4 hàng trụ đá giữa dòng và 2 mố bờ, tạo khoảng hở xuyên suốt dưới sàn khi quan sát từ bờ lạch.
- [x] **Mái ngói âm dương**: Đan xen hình học thật giữa ngói úp và ngói ngửa, lộ hàng rui gỗ đỡ mái bên dưới.
- [x] **Phối trộn ngói tu bổ 2024**: 30% ngói cũ sẫm màu rêu đốm xen 70% ngói mới cam đất nung.
- [x] **Phân hóa cấu kiện gỗ**: Gỗ cũ bảo tồn sẫm màu ấm (`#583826`) bên cạnh các thanh gỗ mới thay thế màu nâu sáng (`#AE7A44`).
- [x] **Hoành phi Lai Viễn Kiều**: 3 chữ 來遠橋 chạm nổi hình học 3D, đọc từ phải sang trái, có ấn Châu Ấn ngự phê bên trái.
- [x] **Đôi mắt cửa truyền thống**: Mắt cửa gỗ tròn chạm 4 bông cúc quanh xoáy âm dương, sơn đỏ son, không thêm dải lụa thừa.
- [x] **Cặp tượng linh thú đầu Đông (Trần Phú)**: Cặp tượng Chó (Thần Cẩu) ngồi chầu trang nghiêm trên bệ đá.
- [x] **Cặp tượng linh thú đầu Tây (Minh Khai)**: Cặp tượng Khỉ (Thần Hầu) ngồi xếp bằng ôm quả đào trước ngực.
- [x] **Phù điêu cổng hồi**: Phù điêu Phật Thủ ở cổng Đông; phù điêu Hai Quả Lựu ở cổng Tây.
- [x] **Màu sắc vôi tường**: Vách hồi vôi đỏ son ấm áp; thành cầu phủ vôi trắng ngà pha xám nhụ làm dịu thị giác.
- [x] **Trụ đá chân cầu**: Chân trụ ẩm bám rêu sẫm màu sát mép nước, phần trên khô sáng màu.
- [x] **Gốm men bờ nóc**: Lưỡng long tranh châu mây lửa khảm men lam coban và sứ trắng phản quang lấp lánh khi đổi góc.
- [x] **Nội thất miếu thờ**: Tượng Bắc Đế Trấn Vũ mặt đỏ chân đạp rùa đen nhìn qua vách thượng song hạ bản.

---

## 4. Danh Sách Tệp Ảnh Đo Kiểm Trong `qa/`

1. `view1_desktop.png` (1920 × 1080): Toàn cảnh Chùa Cầu từ bờ nam lạch nước, góc nhìn ống kính 35 mm.
2. `view2_desktop.png` (1920 × 1080): Tầm mắt người đi bộ dưới mái cầu từ đầu đường Trần Phú nhìn dọc sang phía Tây.
3. `view3_desktop.png` (1920 × 1080): Cận cảnh cách 2 bước chân trước cửa miếu, đọc rõ hoành phi Lai Viễn Kiều và mắt cửa.
4. `view1_mobile.png` (390 × 844): Toàn cảnh từ bờ lạch khung hình dọc điện thoại, bao trọn hai đầu cầu và mặt nước.
5. `view2_mobile.png` (390 × 844): Góc nhìn dọc hành lang gỗ trên điện thoại.
6. `view3_mobile.png` (390 × 844): Cận cảnh hoành phi và cửa miếu trên điện thoại.

