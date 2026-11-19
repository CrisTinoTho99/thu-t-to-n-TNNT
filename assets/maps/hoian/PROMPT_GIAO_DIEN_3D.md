# Prompt chỉnh giao diện và trải nghiệm 3D Chùa Cầu Hội An

Hãy chỉnh trực tiếp demo C++/OpenGL 3.3 trong `assets/maps/hoian` của dự án Virtual Landmark Tour. Người xem cần cảm nhận được một buổi sáng ở Chùa Cầu khi bước từ nắng vào bóng mái, dịch sang bên để nhìn nét chạm trên hoành phi, rồi xuống sát mặt lạch nhìn bóng cầu lay động. Dùng mô hình, shader và bốn góc nhìn đang có làm nền. Đối chiếu chi tiết kiến trúc và màu sau tu bổ với `SOURCES.md`.

## Giao diện khi tham quan

- Để cảnh 3D chiếm trọn cửa sổ. Góc trên trái chỉ hiện tên điểm đang đứng, lấy từ `map.json`. Dọc mép dưới đặt bốn nút **1 Bờ lạch · 2 Hành lang · 3 Cửa miếu · 4 Chân cầu**. Dùng chữ sáng có dấu tiếng Việt, nền nâu sẫm đủ tương phản và một nét vàng mảnh đánh dấu góc đang chọn. Ở khung dọc, thu gọn thành một nút hiện tên góc và mở danh sách khi nhấn.
- Cho phép bấm nút hoặc phím **1–4**. Khi đổi góc, làm mờ khung hình cũ rồi hiện góc mới trong khoảng 0,3 giây. Giữ **WASD** để đi, **chuột phải** để nhìn quanh. Thêm phím **H** để ẩn hoặc hiện toàn bộ giao diện khi ngắm cảnh. Gợi ý thao tác xuất hiện ngắn ở lần mở đầu.
- Khi đến gần và nhìn vào một điểm có trong `hotspots`, chỉ hiện tên chi tiết đó cùng gợi ý **E · Xem chi tiết**. Nhấn E mở mô tả tiếng Việt từ bản đồ. Chỉ hiện gợi ý cho vật nằm trong tầm nhìn trực tiếp. Đặt thẻ sát mép màn hình để người xem vẫn thấy vật mình đang đọc. Esc đóng thẻ trước, khi thẻ đã đóng thì Esc thoát demo.

## Cảnh 3D và chuyển động

| Góc | Chỉnh trong cảnh | Dấu hiệu đạt khi chạy thử |
|---|---|---|
| **1 · Bờ lạch** | Chiếu nắng sớm xiên qua mép mái. Giảm độ gắt của các viên ngói liền kề, cho men lam trên ba ô bờ nóc bắt sáng nhẹ. Tán lá phía xa lay chậm với biên độ nhỏ, chân cành đứng yên. | Nhìn được đường cong của nóc và đủ hai đầu cầu. Khi đứng yên, chỉ lá và nước chuyển động nhẹ. |
| **2 · Hành lang** | Tạo bóng tiếp xúc ở chỗ cột chạm bệ đá. Cho vân gỗ chạy theo thớ từng cấu kiện, tay vịn có vệt bóng ở mặt trên. Bổ sung chiều sâu ở những mặt tiền nhìn xuyên qua cổng bằng hiên nhô, cửa lùi và bậc thềm. | Bước ngang nửa mét sẽ thấy cột gần che rồi để lộ dãy nhà xa. Trong bóng mái vẫn phân biệt được tay vịn, ván sàn và đầu mộng. |
| **3 · Cửa miếu** | Giữ góc nhìn hơi xiên. Dùng ánh sáng hắt dịu để mép vát ba chữ thếp vàng bắt sáng khi người xem bước ngang. Giữ mắt cửa, song gỗ và bàn thờ thành ba lớp nhìn rõ. | Đọc được cả ba chữ. Khi dịch sang bên, ánh sáng đổi trên cạnh chữ và song cửa lần lượt che một phần bàn thờ. |
| **4 · Chân cầu** | Tinh chỉnh phản chiếu và gợn nước hiện có. Cho gợn tách quanh mũi trụ rồi yếu dần theo khoảng cách. Đường đá ẩm cao thấp theo từng khối, bóng dưới sàn cầu đậm hơn vùng nước ngoài nắng. | Nhìn được khoảng hở giữa các trụ. Bóng cầu vỡ nhẹ gần chân trụ và còn nhận ra được ở vùng nước xa. |

## Bàn giao

Biên dịch `hoian_tour`, đi thử cả bốn góc bằng bàn phím và chuột, kiểm tra mở/đóng thẻ chi tiết, rồi chạy bộ kiểm thử hiện có. Xuất ảnh bốn góc ở 1920 × 1080 và 390 × 844. Chụp thêm một khung có giao diện và một khung cùng vị trí sau khi nhấn H để đối chiếu. Đo FPS trước và sau trên cùng máy, cùng góc nhìn và độ phân giải. Ghi rõ số đo cùng các phần còn hạn chế.
