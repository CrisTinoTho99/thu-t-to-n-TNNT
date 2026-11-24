# Prompt dựng Tượng Nữ thần Tự do

Dựng một map 3D cho dự án Virtual Landmark Tour bằng cách tái hiện Tượng Nữ thần Tự do trên đảo Liberty, New York. Bàn giao mô hình GLB có thể đi vòng quanh và chọn từng bộ phận trong ứng dụng. Làm việc trong `assets/maps/liberty/`.

**Dáng tượng.** Đối chiếu ảnh chính diện, ảnh nghiêng và ảnh từ phía sau trước khi dựng. Giữ đúng hai mốc tỷ lệ: từ mặt trên của bệ đến đầu đuốc khoảng 46 m, từ mặt đất đến đầu đuốc khoảng 93 m. Tay phải của tượng giơ cao ngọn đuốc. Tay trái ôm tấm bảng nghiêng sát thân, khắc “JULY IV MDCCLXXVI”. Nhìn từ cạnh bên phải phải thấy gót chân phải nhấc lên và áo choàng rủ theo bước chân. Dựng các nếp vải lớn từ vai xuống thân bằng hình khối thật để chúng đổ sắc độ khác nhau khi người xem đổi góc.

**Chi tiết và màu.** Tạo khuôn mặt trầm tĩnh, sống mũi rõ và mắt nhìn về phía cảng. Vương miện có bảy tia nhọn và một dải 25 ô cửa nhỏ. Tách đường viền các tia để vẫn đếm được ở góc nhìn thích hợp. Thân tượng mang màu xanh patina của đồng phong hóa, biến thiên nhẹ ở các nếp áo. Riêng lửa đuốc mang màu vàng lá. Dựng xích đứt sát chân, bệ đá granite nhiều tầng và phần nền Fort Wood hình sao 11 cánh. Đặt camera mở đầu chếch từ phía trước, lấy trọn bệ và đuốc. Bổ sung một góc sau để quan sát bàn chân và xích.

**Tích hợp.** Xuất `models/liberty.glb`, dùng mét, trục Y hướng lên và hướng trước là -Z. Ghi màu bề mặt vào thuộc tính đỉnh `COLOR_0` và xuất pháp tuyến để màu đồng, đá và vàng hiển thị trong renderer hiện tại. Tách các mesh tương tác thành những node riêng như `L01_Pedestal`, `L02_Robe`, `L03_Crown`, `L04_Torch`, `L05_Tablet`. Tạo `map.json` theo cấu trúc của map Eiffel với `id` là `liberty`, mô tả tiếng Việt, camera, điểm quan sát, đường tham quan và hotspot. Mỗi `parts[].node` phải khớp chính xác một node có mesh trong GLB. Ghi nguồn và giấy phép của tài sản bên ngoài vào `SOURCES.md`.

**Kiểm tra.** Mở map bằng phím 2 trong menu. Ở góc nhìn ban đầu, người xem phải phân biệt được ngọn đuốc, vương miện và tấm bảng. Đi vòng ra sau để kiểm tra mặt lưng, rồi nhấp thử ngọn đuốc và tấm bảng để xác nhận ứng dụng hiện đúng mô tả.

Tư liệu đối chiếu: [kích thước, vương miện và đuốc – National Park Service](https://home.nps.gov/stli/learn/historyculture/statue-statistics.htm), [tư thế và chất liệu – National Park Service](https://www.nps.gov/stli/learn/statue-of-liberty-facts.htm), [nền Fort Wood – National Park Service](https://www.nps.gov/npnh/learn/news/fact-sheet-stli.htm).
