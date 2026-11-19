# Prompt dựng Chùa Cầu Hội An cho Virtual Landmark Tour (Kiểu 3D)

Dựng một map 3D cho dự án Virtual Landmark Tour bằng cách tái hiện di tích Chùa Cầu (Lai Viễn Kiều) tại Hội An, Quảng Nam, theo diện mạo sau đợt tu bổ toàn diện năm 2024. Bàn giao mô hình GLB thời gian thực có thể đi xuyên qua (walkthrough) và nhấp chọn từng bộ phận trong ứng dụng. Tập trung toàn bộ tệp trong `assets/maps/hoian/`.

**Khối kiến trúc và tỷ lệ.** Lấy chiều dài tổng thể khoảng 18 m và bề rộng cầu khoảng 3 m làm mốc tỷ lệ. Dựng khung gỗ bảy gian với tám hàng vì kèo truyền thống, sàn ván vồng nhẹ với độ vồng đỉnh nhịp giữa nâng cao khoảng 34 cm. Năm nhịp nước giữa tựa trên hệ trụ đá và hai mố cầu, để lộ khoảng hở thông suốt dưới sàn khi quan sát từ bờ lạch. Miếu thờ Bắc Đế Trấn Vũ nối vuông góc vào giữa cầu ở phía bắc tạo thành mặt bằng chữ T đặc trưng. Mái dốc võng thấp lợp ngói âm dương úp ngửa đan xen bằng hình học thật, có lớp rui gỗ đỡ phía dưới hiên. Hai đầu cầu có cổng hồi vôi đỏ son, cửa vòm dẫn ra phố cổ.

**Chi tiết, linh thú và hoành phi.** Đặt hai cặp tượng linh thú bằng gỗ ngự trên bệ đá ở hai đầu lối vào theo đúng tư liệu di sản: cặp tượng Khỉ (Thần Hầu) ở đầu cầu phía Đông (đường Trần Phú) và cặp tượng Chó (Thần Cẩu) ở đầu cầu phía Tây (đường Nguyễn Thị Minh Khai) trong tư thế ngồi chầu hướng vào lòng cầu. Ở giữa cầu, quay mặt về cửa miếu Bắc Đế Trấn Vũ: thể hiện rõ biển hoành phi “Lai Viễn Kiều” (來遠橋) sơn son thếp vàng với nét chữ chạm nổi đọc từ phải sang trái theo đúng ảnh hiện vật, cùng đôi mắt cửa (môn thần) tròn gắn dải lụa đỏ phía dưới.

**Màu sắc và chất liệu sau tu bổ 2024.** Lấy kết quả tu bổ năm 2024 làm chuẩn màu:
- Khung gỗ: Giữ sự phân hóa trực quan giữa các cấu kiện gỗ cổ sẫm màu được bảo tồn và những thanh gỗ mới thay thế màu nâu sáng hơn. Bề mặt tay vịn nhẵn bóng ở tầm bàn tay người vịn, vân gỗ chạy dọc thân xà và đổi hướng ở mặt cắt đầu gỗ.
- Mái và bờ nóc: Ngói đất nung có độ nhám lì bắt sáng mờ; họa tiết rồng mây gốm men lam - trắng (cobalt & white) trên bờ nóc có độ bóng lóa phản quang khi thay đổi góc nhìn.
- Tường và đá: Vách miếu và cổng hồi mang sắc vôi đỏ son ấm áp; trụ đá mố cầu phân biệt rõ mảng ẩm bám rêu sẫm màu sát mép nước và mảng khô sáng màu phía trên.
- Ánh sáng: Bối cảnh nắng sớm chiếu xiên mềm mại vừa chạm mép mái nam, đổ bóng qua hàng cột xuống sàn ván và gợn nước lạch.

**Tích hợp kỹ thuật vào Virtual Landmark Tour.** Xuất `models/chua_cau.glb` theo đơn vị mét, trục Y hướng lên, hướng trước là -Z. Xuất pháp tuyến (Normal) và ghi màu vật liệu vào thuộc tính đỉnh `COLOR_0` để hiển thị trong shader của ứng dụng. Tách mô hình thành các node có mesh riêng biệt tương ứng với `parts` trong `map.json`:
- `H01_Masonry`: Hệ trụ đá và mố cầu
- `H02_Deck`: Sàn ván cầu vồng nhẹ
- `H03_Timber`: Khung cột, xà, vì kèo gỗ và rui đỡ mái
- `H04_Rails`: Tay vịn và hàng con tiện
- `H05_Portals`: Cổng hồi hai đầu cầu và cặp tượng Thần Hầu / Thần Cẩu
- `H06_RoofTiles`: Mái ngói âm dương hình học thật
- `H07_Ceramics`: Họa tiết gốm men bờ nóc và bờ dái
- `H08_Shrine`: Khối kiến trúc miếu thờ Bắc Đế Trấn Vũ
- `H09_Plaque`: Hoành phi Lai Viễn Kiều chạm nổi
- `H10_DoorEyes`: Đôi mắt cửa truyền thống

Thiết lập `map.json` với `id: "hoian"`, siêu dữ liệu tiếng Việt, camera khởi đầu và đường đi tham quan xuyên qua cầu (`tour_path`). Ghi hồ sơ trích dẫn tài liệu vào `SOURCES.md`.

**Kiểm tra trong ứng dụng.** Mở demo và kiểm tra ba góc nhìn tiêu chuẩn:
1. *Góc toàn cảnh từ bờ lạch (Phím 1)*: Đặt máy cao 1,6 m ở bờ nam, góc nhìn tương đương ống kính 35 mm, lấy trọn hai đầu cầu, dải trời sớm mỏng phía trên và khoảng nước gợn sóng phản chiếu bóng trụ đá phía dưới.
2. *Góc dưới mái cầu (Phím 2)*: Tầm mắt người đi bộ từ đầu đường Trần Phú, quan sát hàng cột lướt qua, ánh sáng lướt trên tay vịn nhẵn và hàng tượng Thần Cẩu ở cổng phía Tây.
3. *Góc trước cửa miếu (Phím 3)*: Đặt máy cách 2 bước chân chính diện cửa miếu, đọc rõ ba chữ “Lai Viễn Kiều” và quan sát trọn vẹn đôi mắt cửa cùng dải lụa đỏ.

**Tư liệu đối chiếu:**
- [Hồ sơ kiến trúc và ảnh Chùa Cầu – Trung tâm Quản lý Bảo tồn Di sản Văn hóa Hội An](https://hoianheritage.net/vi/di-san-van-hoa/Cau/Chua-Cau-134.html)
- [Kích thước, nhịp cầu và chi tiết cửa miếu](https://hoianheritage.net/vi/bao-chi/bao-chi-ve-di-san/chua-cau-404.html)
- [Tư liệu kiến trúc về vị trí miếu và hai đầu cầu](https://hoianheritage.net/vi/trao-doi-chuyen-nganh/chuyen-de-nghien-cuu-trao-doi/mot-so-thong-tin-ve-kien-truc-chua-cau-qua-tu-lieu-994.html)
- [Báo cáo kết quả dự án tu bổ Chùa Cầu năm 2024](https://hoianheritage.net/vi/trao-doi-chuyen-nganh/chuyen-de-nghien-cuu-trao-doi/thong-tin-ve-ket-qua-tu-bo-di-tich-chua-cau-1242.html)

