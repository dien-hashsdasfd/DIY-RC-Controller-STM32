// ====================================================================
// THIẾT KẾ VỎ TAY CẦM ĐIỀU KHIỂN RC / ROBOT CHUẨN DATASHEET THỰC TẾ 100%
// - ĐỐI SOÁT CHÍNH XÁC TỪNG MILIMÉT THEO LINH KIỆN THỰC TẾ CỦA BẠN:
//   1. Màn hình LCD 1604A (WH1604A):
//      + PCB: 87.0 x 60.0 x 1.6 mm | 4 lỗ ốc góc: tâm 82.0 x 55.0 mm (Ø2.5mm, cách mép 2.5mm)
//      + Khung viền kim loại đen: 77.0 x 40.0 mm | Vùng kính hiển thị (VA): 62.0 x 26.0 mm
//      + Độ dày cụm đèn nền LED + khung sắt trên mặt PCB: 7.5 mm (Trụ đỡ nắp dài chuẩn 5.0 mm)
//   2. 2x Cảm biến Joystick PS2 (HW-504 / KY-023):
//      + PCB: 26.0 x 34.0 x 1.6 mm | 4 lỗ ốc góc: tâm 20.0 x 27.0 mm (Ø3.0mm)
//      + Hộp Gimbal: 16.0 x 16.0 x 11.0 mm + 2 biến trở 10k + công tắc nhấn SW bên hông
//      + Vòm cầu che bụi Ø21.5mm & núm nấm cao su Ø25.0mm (Trụ đỡ nắp dài chuẩn 7.0 mm)
//   3. Chiết áp xoay WH148 (B10K):
//      + Bầu thân Ø16.0 x 8.5 mm, cổ ren M7x0.75 (lỗ khoét Ø7.2mm), 3 chân hàn pitch 5.0mm
//      + Đai ốc M7 + long đen siết sát mặt trên nắp, chụp núm nhôm đen vạch trắng Ø15x13.5mm
//   4. Nút nguồn kim loại Inox 16mm có LED biểu tượng Power (Nhấn giữ):
//      + Thân ren M16x1 (lỗ khoét Ø16.2mm), vành mũ Inox Ø18.8mm áp phẳng mặt nắp
//      + Gioăng cao su + Đai ốc lục giác M16 siết sát mặt dưới nắp (Z = 0)
//   5. Module RF LoRa AS32-TTL-100 / Ebyte E22-400T30D (SMA-K):
//      + PCB: 21.0 x 36.0 x 1.6 mm (khe đỡ tương thích tới 24 x 43 mm), 7 chân DIP 2.54mm
//      + Cổ ren đồng SMA-K Ø6.35mm xuyên trực tiếp qua lỗ Ø6.6mm thành trên, khóa tán SMA ngoài
//   6. Kit STM32F103C8T6 Blue Pill:
//      + PCB: 53.34 x 22.86 x 1.6 mm | 2 hàng rào 20P khoảng cách chuẩn 15.24mm (0.6 inch)
//   7. Khay 2 Pin 18650 (2S Holder):
//      + Kích thước chuẩn: 76.0 x 40.5 x 19.5 mm
//   8. Mạch hạ áp Mini360 Buck Converter:
//      + Kích thước chuẩn: 17.5 x 12.0 x 1.6 mm (đặt tại khoang riêng [34, 46] không chạm pin)
// - MẶT TRÊN PHẲNG SẠCH 100%: Không vẽ gờ thừa, không lộ ốc mặt tiền, bắt ốc chìm từ đáy.
// ====================================================================

$fn = 24; // Độ mịn bề mặt chuẩn CAD

// --- KÍCH THƯỚC HÌNH HỌC TỔNG THỂ VỎ HỘP (mm) ---
cao_tong     = 52.0;  // Chiều cao tổng thể khi đóng nắp
day_nap      = 3.5;   // Chiều dày tấm nắp trên (Z = 0 đến Z = +3.5mm)
day_thanh    = 2.5;   // Chiều dày thành vỏ đặc kín xung quanh
day_day      = 2.5;   // Chiều dày sàn đáy hộp
h_day        = cao_tong - day_nap; // 41.5mm (chiều cao thân đáy)

// --- TỌA ĐỘ CHUẨN CỦA CÁC LINH KIỆN TRÊN VỎ (mm) ---
lcd_x = 0.0;    lcd_y = 12.0;    // Tâm màn hình LCD 1604A (ụ nổi nghiêng 15° trung tâm)
goc_nghieng_lcd = 15.0;         // Góc nghiêng màn hình LCD nhô chéo lên (15 độ hướng về mắt người điều khiển)
lcd_z = 9.0;                    // Cao độ tâm xoay cụm màn hình LCD (mm)
joy_x = 76.0;   joy_y = -8.0;   // Tâm 2 cụm Joystick PS2 trái/phải (hạ nhẹ 3mm để vai trên cực rộng thoáng)

// BỐ CỤC 2 VAI ĐỐI XỨNG CHUẨN TAY CẦM FPV DRONE & XE Ô TÔ RC:
// - Vai Phải (ngay trên Joystick Phải): Biến trở WH148 (chỉnh dải xung PWM tốc độ nhanh/chậm cho xe RC) + Công tắc SW-C
// - Vai Trái (ngay trên Joystick Trái): 2 Công tắc gạt MTS-102 (SW-A: Arm/Khóa an toàn, SW-B: Mode bay/Mode chạy xe)
pot_x = 59.5;   pot_y = 33.0;   // Biến trở WH148 chỉnh tốc độ PWM ngay tầm ngón cái/ngón trỏ phải
sw_coords = [
    [-84.5, 33.0],              // SW-A (Vai trái ngoài: Arm/Disarm Drone | Khóa/Phanh Xe RC)
    [-59.5, 33.0],              // SW-B (Vai trái trong: Flight Mode Drone | Số Nhanh/Chậm/Lùi Xe RC)
    [ 84.5, 33.0]               // SW-C (Vai phải ngoài: Đèn LED / Còi Beeper / Chuyển Mode Drone-Xe)
];
pwr_x = 0.0;    pwr_y = -64.0;  // Tâm nút nguồn kim loại 16mm
bat_x = 0.0;    bat_y = -28.5;  // Tâm khay 2 pin 18650 (Y từ -45.75 đến -5.25)
stm_x = 0.0;    stm_y = 13.0;   // Tâm bo mạch STM32 Blue Pill dưới sàn đáy (Y từ +5.0 đến +28.0, cách khay pin 10.25mm)
lora_x = 0.0;   lora_y = 56.0;  // Tâm bo mạch LoRa AS32/E22 (Y từ +32.0 đến sát thành trong +68.0)
lora_z = 18.0;                  // Cao độ mặt dưới bo LoRa (tâm giắc SMA tại Z = 16.5mm)
buck_x = 28.0;  buck_y = 41.5;  // Tâm mạch hạ áp Mini360 (khoang riêng bên phải, nhường chỗ trên cho Buzzer)
tp_x   = -26.0; tp_y   = 60.5;  // Tâm mạch sạc pin TP4056 Type-C có bảo vệ (26.0 x 17.0 mm, cổng Type-C xuyên thành trên)
buz_x  = 26.5;  buz_y  = 67.5;
buz_z  = day_day + 8.0;  // Tâm còi báo Buzzer 5V Ø12x9.5mm (đối xứng với mạch sạc TP4056 qua LoRa)
trim_coords = [[-29.0, -41.0], [0.0, -38.0], [29.0, -41.0]]; // 3 Nút nhấn phi 16 có đèn 4 chân xếp vòng cung công thái học dưới màn hình LCD                // Toạ độ Y của 3 nút nhấn Menu / Trim (giữa gờ màn hình LCD và nút nguồn)

// Vị trí 6 trụ ốc M3 đóng/mở vỏ hộp (Bắt ngược từ mặt đáy lên trụ ren ngầm của nắp, cách thành trong >2.5mm)
tru_ocs = [
    [-42.5,  62.0], [ 42.5,  62.0], // 2 trụ vai trên thân giữa (ngoài phạm vi bo LCD Y=+45)
    [-41.5, -65.0], [ 41.5, -65.0], // 2 trụ chuôi dưới thân giữa (2 bên nút nguồn, dưới khay pin)
    [-98.0,  -5.0], [ 98.0,  -5.0]  // 2 trụ ngoài báng cầm (ngoài phạm vi bo Joystick X=+-89)
];

// --------------------------------------------------------------------
// 1. ĐƯỜNG BIÊN 2D CÔNG THÁI HỌC (LIỀN KHỐI, ÔM SÁT BỐ CỤC LINH KIỆN)
// --------------------------------------------------------------------
module ProfileTayCam2D(delta_offset = 0) {
    // Dịch chuyển song song toàn bộ biên dạng mượt (đảm bảo thành vỏ dày đều 2.5mm 100% tại mọi điểm)
    offset(r = delta_offset) {
        // Bo mượt các góc lõm giao giữa báng cầm và thân giữa với bán kính R = 8mm
        offset(r = -8) offset(r = 8) {
            // Bo mượt các góc lồi ngoài với bán kính R = 8mm
            offset(r = 8) {
                union() {
                    // 1. Thân giữa (Console trung tâm: Rộng 100mm [-50..+50], Dài 140mm [-70..+70])
                    hull() {
                        translate([-42,  68]) square([1, 1], center = true);
                        translate([ 42,  68]) square([1, 1], center = true);
                        translate([-42, -72]) square([1, 1], center = true);
                        translate([ 42, -72]) square([1, 1], center = true);
                    }

                    // 2. Báng cầm & Vai điều khiển bên trái (Left FPV Grip & Shoulder Pod)
                    hull() {
                        translate([-38,  44]) circle(r = 6);
                        translate([-88,  44]) circle(r = 6);
                        translate([-joy_x, joy_y]) circle(r = 16);
                        translate([-94,  -8]) circle(r = 8);
                        translate([-74, -58]) circle(r = 10);
                        translate([-38, -58]) circle(r = 10);
                    }

                    // 3. Báng cầm & Vai điều khiển bên phải (Right FPV Grip & Shoulder Pod)
                    hull() {
                        translate([ 38,  44]) circle(r = 6);
                        translate([ 88,  44]) circle(r = 6);
                        translate([ joy_x, joy_y]) circle(r = 16);
                        translate([ 94,  -8]) circle(r = 8);
                        translate([ 74, -58]) circle(r = 10);
                        translate([ 38, -58]) circle(r = 10);
                    }
                }
            }
        }
    }
}

module KhoiKhung3D(cao, delta_offset = 0) {
    linear_extrude(height = cao, convexity = 10)
        ProfileTayCam2D(delta_offset);
}

// ====================================================================
// BỘ THƯ VIỆN LINH KIỆN 3D CHUẨN DATASHEET 100% (KHÔNG VẼ THỪA)
// ====================================================================

// 1. MÀN HÌNH LCD 1604A (WH1604A - 87.0 x 60.0 mm, tâm lỗ 82.0 x 55.0 mm)
module LinhKien_LCD1604A() {
    // Bo mạch FR-4 xanh lá (87.0 x 60.0 x 1.6 mm) áp sát mặt dưới 4 trụ đỡ tại Z = -5.0 đến -6.6mm
    color([0.08, 0.48, 0.20])
        translate([0, 0, -5.8])
        difference() {
            cube([87.0, 60.0, 1.6], center = true);
            // 4 Lỗ bắt vít góc Ø2.5mm chuẩn Datasheet (tâm +-41.0 x +-27.5mm, cách mép 2.5mm)
            for (dx = [-41.0, 41.0], dy = [-27.5, 27.5])
                translate([dx, dy, 0]) cylinder(h = 3.0, r = 1.25, center = true);
            // 16 Lỗ chân hàn chuẩn 2.54mm ở mép trên bên trái (chân 1..16 từ X=-35.0 đến X=+3.1mm)
            for (i = [0:15])
                translate([-35.0 + i * 2.54, 27.5, 0]) cylinder(h = 3.0, r = 0.5, center = true);
        }

    // 4 Vòng đồng mạ vàng quanh lỗ ốc (Ø4.4mm < 5.0mm nên nằm gọn 100% trong góc bo mạch, không tràn mép)
    color([0.88, 0.74, 0.28])
        for (dx = [-41.0, 41.0], dy = [-27.5, 27.5]) {
            translate([dx, dy, -5.0])
                difference() {
                    cylinder(h = 0.06, r = 2.2, center = true);
                    cylinder(h = 0.2, r = 1.25, center = true);
                }
            translate([dx, dy, -6.6])
                difference() {
                    cylinder(h = 0.06, r = 2.2, center = true);
                    cylinder(h = 0.2, r = 1.25, center = true);
                }
        }

    // 4 Vít Inox M2.5x8mm vặn từ mặt lưng bo mạch (Z = -8.2..-6.6) xuyên lên 4 trụ đỡ của nắp
    color([0.82, 0.84, 0.88]) {
        for (dx = [-41.0, 41.0], dy = [-27.5, 27.5]) {
            // Đầu mũ ốc M2.5 (Ø4.4mm nằm gọn trên vòng đồng, không bao giờ thò ra ngoài mép PCB)
            translate([dx, dy, -8.2])
                difference() {
                    cylinder(h = 1.6, r1 = 2.0, r2 = 2.2);
                    translate([0, 0, -0.4]) cylinder(h = 1.1, r = 1.0, $fn = 6);
                }
            // Thân ren M2.5 xuyên qua PCB ăn vào trụ đỡ nắp
            translate([dx, dy, -6.6])
                cylinder(h = 7.5, r = 1.2);
        }
    }

    // Hàng 16 chân Header 2.54mm ở mép trên bên trái (X = -35.0 đến +3.1mm, Y = +27.5mm, quay xuống lòng hộp)
    color([0.15, 0.15, 0.15])
        translate([-15.95, 27.5, -7.85])
        cube([40.64, 2.5, 2.5], center = true);
    color([0.88, 0.76, 0.28])
        for (i = [0:15])
            translate([-35.0 + i * 2.54, 27.5, -8.8])
            cube([0.64, 0.64, 8.5], center = true);

    // 2 Chip IC điều khiển HD44780 đổ keo Epoxy đen (COB blobs) ở mặt lưng bo mạch (Z = -6.6..-7.8)
    color([0.10, 0.10, 0.12]) {
        translate([-18.0, 0, -6.6]) scale([1, 1, 0.22]) sphere(r = 6.5);
        translate([ 18.0, 0, -6.6]) scale([1, 1, 0.22]) sphere(r = 6.5);
    }

    // Khối tản sáng đèn nền LED trắng (74.0 x 38.5 x 4.2 mm, có vấu A/K bên phải như ảnh thật)
    color([0.94, 0.95, 0.97]) {
        translate([0, 0, -2.9])
            cube([74.0, 38.5, 4.2], center = true);
        // Phần vấu tản sáng LED nhô sang cạnh phải (chân A - K)
        translate([38.0, 0, -2.9])
            cube([3.5, 16.0, 4.0], center = true);
    }

    // Khung kim loại dập sơn đen (77.0 x 40.0 x 7.5 mm, từ mặt PCB Z=-5.0 lên Z=+2.5mm lọt khít cửa sổ nắp)
    color([0.12, 0.13, 0.15])
        translate([0, 0, -1.25])
        difference() {
            cube([77.0, 40.0, 7.5], center = true);
            // Cửa sổ Viewing Area chuẩn Datasheet: 62.0 x 26.0 mm
            translate([0, 0, 2.0]) cube([62.0, 26.0, 5.0], center = true);
            // Khoét rỗng lòng dưới khung sắt ôm khối đèn nền
            translate([0, 0, -1.0]) cube([74.2, 38.6, 6.5], center = true);
        }

    // 6 Ngàm thép đen gập khóa khung ở mặt sau PCB (đúng vị trí thực tế)
    color([0.15, 0.16, 0.18]) {
        for (tx = [-28, 0, 28], ty = [-19.5, 19.5])
            translate([tx, ty, -6.7]) cube([3.5, 1.4, 0.4], center = true);
    }

    // Mặt kính tinh thể lỏng LCD xanh vàng (62.0 x 26.0 x 1.2 mm)
    color([0.18, 0.65, 0.22])
        translate([0, 0, 1.4])
        cube([62.0, 26.0, 1.2], center = true);
}

// 2. CẢM BIẾN JOYSTICK PS2 (HW-504 / KY-023: PCB 26.0 x 34.0 mm, tâm lỗ 20.0 x 27.0 mm, tổng cao 32mm)
// Hỗ trợ tham số goc_gat (độ) để kiểm tra hành trình gạt thực tế quanh tâm trục xoay Gimbal (Z = -2.5mm)
module LinhKien_JoystickPS2(goc_gat_y = 0) {
    // Bo mạch PCB đen (26.0 x 34.0 x 1.6 mm) áp vào 4 trụ đỡ dài 9.0mm (mặt trên PCB tại Z = -9.0mm, mặt dưới tại Z = -10.6mm)
    color([0.11, 0.12, 0.14])
        translate([0, 0, -9.8])
        difference() {
            cube([26.0, 34.0, 1.6], center = true);
            // 4 Lỗ ốc góc Ø3.0mm chuẩn (tâm 20.0 x 27.0 mm -> ox = +-10.0, oy = +-13.5)
            for (ox = [-10.0, 10.0], oy = [-13.5, 13.5])
                translate([ox, oy, 0]) cylinder(h = 3.0, r = 1.5, center = true);
        }

    // 4 Vòng thiếc mạ quanh lỗ ốc (Ø5.0mm nằm gọn trong bo 26x34mm)
    color([0.82, 0.84, 0.86])
        for (ox = [-10.0, 10.0], oy = [-13.5, 13.5]) {
            translate([ox, oy, -9.0])
                difference() {
                    cylinder(h = 0.06, r = 2.5, center = true);
                    cylinder(h = 0.2, r = 1.5, center = true);
                }
            translate([ox, oy, -10.6])
                difference() {
                    cylinder(h = 0.06, r = 2.5, center = true);
                    cylinder(h = 0.2, r = 1.5, center = true);
                }
        }

    // 4 Vít Inox M3x8mm siết từ mặt lưng bo mạch Joystick (Z = -12.4..-10.6) lên 4 trụ nắp
    color([0.82, 0.84, 0.88]) {
        for (ox = [-10.0, 10.0], oy = [-13.5, 13.5]) {
            translate([ox, oy, -12.4])
                difference() {
                    cylinder(h = 1.8, r1 = 2.2, r2 = 2.5);
                    translate([0, 0, -0.4]) cylinder(h = 1.2, r = 1.15, $fn = 6);
                }
            translate([ox, oy, -10.6])
                cylinder(h = 7.5, r = 1.4);
        }
    }

    // Cụm 5 chân cắm Header vuông góc 90 độ ở cạnh dưới (GND, +5V, VRx, VRy, SW)
    color([0.15, 0.15, 0.15])
        translate([0, -14.5, -7.8])
        cube([12.7, 2.5, 2.5], center = true);
    color([0.85, 0.85, 0.90])
        for (i = [-2:2]) {
            translate([i * 2.54, -17.5, -7.8])
                cube([0.64, 6.0, 0.64], center = true);
        }

    // Khung hộp Gimbal thép mạ (16.0 x 16.0 x 11.0 mm) trên mặt PCB (từ Z = -9.0 đến Z = +2.0mm)
    color([0.74, 0.76, 0.80])
        translate([0, 0, -3.5])
        difference() {
            cube([16.0, 16.0, 11.0], center = true);
            translate([0, 0, 1.5]) cube([13.2, 13.2, 9.0], center = true);
        }

    // 2 Biến trở góc 10K gắn sát bên trái (-X) và phía dưới (-Y) hộp Gimbal (tâm xoay tại Z = -2.5mm)
    color([0.14, 0.38, 0.26]) {
        translate([-9.2, 0, -3.0]) cube([2.5, 9.5, 9.5], center = true);
        translate([0, -9.2, -3.0]) cube([9.5, 2.5, 9.5], center = true);
    }
    color([0.9, 0.9, 0.9]) {
        translate([-10.5, 0, -2.5]) rotate([0, 90, 0]) cylinder(h = 0.2, r = 1.8, center = true);
        translate([0, -10.5, -2.5]) rotate([90, 0, 0]) cylinder(h = 0.2, r = 1.8, center = true);
    }

    // Công tắc nhấn SW (Tactile Switch 4.5 x 4.5 mm) nằm bên phải (+X) hộp Gimbal
    color([0.18, 0.18, 0.20])
        translate([9.8, 1.0, -7.0]) cube([4.5, 4.5, 3.8], center = true);
    color([0.75, 0.78, 0.82])
        translate([9.8, 1.0, -4.6]) cylinder(h = 1.2, r = 1.3, center = true);

    // CỤM CẦN GẠT & NÚM NẤM CAO SU XOAY QUANH TÂM TRỤC GIMBAL (Z_pivot = -2.5mm, tức cách mặt PCB 6.5mm chuẩn Datasheet)
    // Tổng chiều cao từ mặt PCB (-9.0mm) lên đỉnh núm nấm (+21.4mm) là đúng 30.4mm (cộng PCB 1.6mm = 32.0mm chuẩn HW-504!)
    translate([0, 0, -2.5])
        rotate([goc_gat_y, 0, 0]) {
            color([0.13, 0.14, 0.16]) {
                // 1. Vòm cầu che bụi Ø21.5mm (r = 10.75mm, nằm ngay trên hộp Gimbal từ Z_rel = +4.7 đến +12.0mm)
                difference() {
                    translate([0, 0, 1.25])
                        sphere(r = 10.75);
                    translate([0, 0, -5.3])
                        cube([26, 26, 20], center = true);
                }
                // 2. Cổ trụ cần gạt (Ø9.2mm thuôn lên Ø8.0mm, từ Z_rel = +10.5 đến +18.5mm)
                translate([0, 0, 10.5])
                    cylinder(h = 8.0, r1 = 4.6, r2 = 4.0);
                // 3. Mũ nấm cao su Ø25.0mm ôm ngón tay cái (từ Z_rel = +18.5 đến +23.9mm -> Z_world = +16.0 đến +21.4mm)
                translate([0, 0, 18.5]) {
                    difference() {
                        union() {
                            cylinder(h = 3.2, r1 = 4.0, r2 = 12.5);
                            translate([0, 0, 3.2])
                                cylinder(h = 2.2, r = 12.5);
                        }
                        translate([0, 0, 5.4 + 14.0 - 1.1])
                            sphere(r = 14.0);
                    }
                }
            }
        }
}

// 3. CHIẾT ÁP XOAY WH148 (B10K - Bầu Ø16.0mm, ren M7, 3 chân pitch 5.0mm, núm nhôm vạch trắng)
module LinhKien_ChietApWH148() {
    // Bầu kim loại Ø16.0 x 8.5 mm áp sát mặt dưới nắp trên (Z = -8.5 đến 0.0mm)
    color([0.78, 0.76, 0.72])
        translate([0, 0, -8.5])
        cylinder(h = 8.5, r = 8.0);

    // Tấm phíp cam dày 1.2mm & 3 chân hàn pitch 5.0mm quay lên (+Y) vào khoang rỗng vai phải
    rotate([0, 0, 180]) {
        color([0.85, 0.45, 0.15])
            translate([0, -7.5, -1.2])
            cube([13.0, 5.0, 1.2], center = true);
        color([0.85, 0.86, 0.90])
            for (i = [-1:1])
                translate([i * 5.0, -11.0, -1.2])
                cube([1.0, 4.5, 0.5], center = true);
    }

    // Cổ ren M7x0.75 (Ø6.8mm, dài 7.0mm từ Z = 0 lên Z = +7.0mm xuyên qua nắp)
    color([0.82, 0.84, 0.88])
        cylinder(h = 7.0, r = 3.4);

    // Long đen phẳng M7 + Đai ốc lục giác M7 nằm TRÊN mặt nắp (Z = day_nap = 3.5mm)
    color([0.88, 0.88, 0.92]) {
        translate([0, 0, day_nap])
            cylinder(h = 0.5, r = 5.5);
        translate([0, 0, day_nap + 0.5])
            cylinder(h = 2.0, r = 5.2, $fn = 6);
    }

    // Trục xoay Ø6.0mm
    color([0.82, 0.84, 0.88])
        translate([0, 0, 7.0])
        cylinder(h = 8.5, r = 3.0);

    // Núm vặn nhôm đen vạch trắng (Ø15.0 x 13.5 mm) chụp lên trục chiết áp (từ Z = day_nap + 1.0)
    translate([0, 0, day_nap + 1.0]) {
        color([0.12, 0.12, 0.14])
            cylinder(h = 13.5, r = 7.5);
        color([0.96, 0.96, 0.98])
            translate([0, 4.5, 7.0])
            cube([1.0, 5.0, 13.6], center = true);
    }
}

// 3B. CÔNG TẮC GẠT MINI MTS-102 (3 CHÂN SPDT ON-ON, CỔ REN M6, CẦN GẠT CHROME CHUẨN TAY DRONE)
module LinhKien_CongTacMTS102(goc_gat = 12) {
    // 1. Thân nhựa xanh dương chuẩn MTS-102 (Rộng X=8.0mm, Dài Y=13.0mm, Cao 7.5mm từ Z=-9.5 đến -2.0mm)
    color([0.12, 0.38, 0.82])
        translate([0, 0, -5.75])
        cube([8.0, 13.0, 7.5], center = true);

    // 2. Nắp khung thép mạ bạc dập chữ ON-ON (8.2 x 13.2 x 2.0 mm từ Z=-2.0 đến 0.0mm áp sát mặt dưới nắp)
    color([0.82, 0.84, 0.88])
        translate([0, 0, -1.0])
        cube([8.2, 13.2, 2.0], center = true);

    // 3. 3 Chân hàn mạ bạc ở đáy (pitch chuẩn 4.7mm dọc trục Y: -4.7, 0, +4.7mm, dài 4.0mm từ Z=-13.5 đến -9.5mm)
    color([0.88, 0.90, 0.94])
        for (py = [-4.7, 0.0, 4.7])
            translate([0, py, -11.5])
            cube([0.8, 2.0, 4.0], center = true);

    // 4. Cổ ren M6x0.75 (Ø6.0mm, xuyên từ Z=0.0 lên Z = day_nap + 3.5 = +7.0mm)
    color([0.78, 0.80, 0.84])
        cylinder(h = day_nap + 3.5, r = 3.0);

    // 5. Long đen chống xoay + Đai ốc lục giác M6 siết trên mặt nắp (Z = day_nap = 3.5mm)
    color([0.86, 0.88, 0.92]) {
        translate([0, 0, day_nap])
            cylinder(h = 0.5, r = 5.0);
        translate([0, 0, day_nap + 0.5])
            cylinder(h = 1.8, r = 4.6, $fn = 6);
    }

    // 6. Cần gạt kim loại mạ Chrome bóng (dài 10.5mm, thuôn từ Ø2.0mm lên đầu tròn Ø3.2mm, gạt góc ±12°)
    color([0.90, 0.92, 0.96])
        translate([0, 0, day_nap + 2.5])
        rotate([goc_gat, 0, 0]) {
            cylinder(h = 9.0, r1 = 1.1, r2 = 1.6);
            translate([0, 0, 9.0])
                sphere(r = 1.6);
        }
}

// 4. NÚT NGUỒN KIM LOẠI INOX 16MM CÓ LED BIỂU TƯỢNG POWER
module LinhKien_NutNguon16mm() {
    // Thân ren Inox M16x1 (Ø16.0mm, từ Z = -20.0mm xuyên qua nắp lên Z = day_nap)
    color([0.82, 0.84, 0.88])
        translate([0, 0, -20.0])
        cylinder(h = 20.0 + day_nap, r = 8.0);

    // Gioăng cao su đen + Đai ốc lục giác M16 siết SÁT MẶT DƯỚI NẮP TRÊN (Z = -3.5 đến 0.0mm)
    color([0.12, 0.12, 0.14])
        translate([0, 0, -1.0])
        cylinder(h = 1.0, r = 9.5);
    color([0.76, 0.78, 0.82])
        translate([0, 0, -3.5])
        cylinder(h = 2.5, r = 10.5, $fn = 6);

    // 5 Chân hàn đồng mạ bạc ở đuôi nút (Z = -25.0 đến -20.0mm)
    color([0.85, 0.78, 0.35])
        for (a = [0:72:288])
            translate([4.8 * cos(a), 4.8 * sin(a), -22.5])
            cube([0.8, 2.0, 5.0], center = true);

    // Vành mũ Inox Ø18.8mm (r = 9.4mm) áp phẳng trực tiếp lên mặt trên nắp (Z = day_nap = 3.5mm)
    translate([0, 0, day_nap]) {
        color([0.86, 0.88, 0.92])
            difference() {
                cylinder(h = 1.8, r1 = 9.4, r2 = 8.8);
                translate([0, 0, -0.2]) cylinder(h = 2.2, r = 6.8);
            }
        // Vòng LED xanh dương
        color([0.15, 0.62, 1.0])
            cylinder(h = 1.9, r = 6.8);
        // Mặt nút bấm trung tâm Inox
        color([0.82, 0.84, 0.88])
            cylinder(h = 2.0, r = 5.2);
        // Biểu tượng Power phát sáng
        color([0.18, 0.75, 1.0]) {
            translate([0, 0, 2.05])
                difference() {
                    cylinder(h = 0.15, r = 3.0, center = true);
                    cylinder(h = 0.4, r = 2.2, center = true);
                    translate([0, 1.8, 0]) cube([1.8, 2.5, 0.5], center = true);
                }
            translate([0, 1.5, 2.05])
                cube([0.6, 2.6, 0.15], center = true);
        }
    }
}

// 5. KHAY CHỨA 2 CELL PIN 18650 (Chuẩn 76.0 x 40.5 x 19.5 mm)
module LinhKien_KhayPin18650() {
    color([0.12, 0.12, 0.14])
        translate([0, 0, 9.75])
        difference() {
            cube([76.0, 40.5, 19.5], center = true);
            translate([0, -9.8, 2.0]) rotate([0, 90, 0]) cylinder(h = 72.0, r = 9.3, center = true);
            translate([0,  9.8, 2.0]) rotate([0, 90, 0]) cylinder(h = 72.0, r = 9.3, center = true);
        }
    // 2 Cell Pin 18650 (Ø18.3 x 65.0 mm)
    color([0.15, 0.52, 0.95]) {
        translate([0, -9.8, 11.5]) rotate([0, 90, 0]) cylinder(h = 65.0, r = 9.15, center = true);
        translate([0,  9.8, 11.5]) rotate([0, 90, 0]) cylinder(h = 65.0, r = 9.15, center = true);
    }
}

// 6. KIT STM32F103C8T6 BLUE PILL (Chuẩn 53.34 x 22.86 x 1.6 mm, 2 hàng chân cách nhau 15.24mm)
module LinhKien_STM32BluePill() {
    // Bo mạch xanh dương (53.34 x 22.86 x 1.6 mm)
    color([0.08, 0.32, 0.75])
        cube([53.34, 22.86, 1.6], center = true);

    // Chip MCU STM32F103C8T6 LQFP-48
    color([0.12, 0.12, 0.14])
        translate([-3.0, 0, 1.4])
        rotate([0, 0, 45])
        cube([7.0, 7.0, 1.4], center = true);

    // Cổng Micro-USB ở mép trái
    color([0.82, 0.85, 0.90])
        translate([-24.5, 0, 2.0])
        cube([5.8, 7.8, 2.8], center = true);

    // Thạch anh 8MHz & 2 hàng Jump Bootloader vàng
    color([0.82, 0.84, 0.88])
        translate([7.5, 0, 1.9])
        cube([10.5, 4.5, 2.4], center = true);
    color([0.95, 0.82, 0.15]) {
        translate([15.5,  2.5, 2.8]) cube([5.0, 2.2, 4.2], center = true);
        translate([15.5, -2.5, 2.8]) cube([5.0, 2.2, 4.2], center = true);
    }

    // 2 Hàng chân rào 20P màu vàng quay LÊN TRÊN (khoảng cách chuẩn 15.24mm -> Y = +-7.62mm)
    color([0.92, 0.80, 0.15]) {
        translate([0,  7.62, 2.05]) cube([50.8, 2.5, 2.5], center = true);
        translate([0, -7.62, 2.05]) cube([50.8, 2.5, 2.5], center = true);
        for (i = [-9.5:9.5]) {
            translate([i * 2.54,  7.62, 4.8]) cube([0.64, 0.64, 6.0], center = true);
            translate([i * 2.54, -7.62, 4.8]) cube([0.64, 0.64, 6.0], center = true);
        }
    }

    // 4 Chân nạp SWD ở mép phải (3V3, DIO, CLK, GND)
    color([0.15, 0.15, 0.15])
        translate([25.0, 0, 2.05]) cube([2.5, 10.16, 2.5], center = true);
    color([0.92, 0.80, 0.15])
        for (i = [-1.5:1.5])
            translate([28.0, i * 2.54, 2.05]) cube([5.0, 0.64, 0.64], center = true);
}

// 7. MODULE RF LORA AS32-TTL-100 / EBYTE E22 (Chuẩn 21.0 x 36.0 mm, giắc đồng SMA-K xuyên thành vỏ)
module LinhKien_LoRaE22() {
    // Bo mạch xanh lá (21.0 x 36.0 x 1.6 mm, từ Y = -18.0 đến Y = +18.0)
    color([0.10, 0.52, 0.25])
        cube([21.0, 36.0, 1.6], center = true);

    // Hộp kim loại chống nhiễu RF (17.5 x 25.0 x 3.0 mm)
    color([0.78, 0.80, 0.84])
        translate([0, -1.0, 2.3])
        cube([17.5, 25.0, 3.0], center = true);

    // Hàng 7 chân cắm DIP 2.54mm ở đuôi module (Y = -16.5mm)
    color([0.15, 0.15, 0.15])
        translate([0, -16.5, -2.0])
        cube([17.78, 2.5, 2.5], center = true);
    color([0.88, 0.76, 0.28])
        for (i = [-3:3])
            translate([i * 2.54, -16.5, -5.2])
            cube([0.64, 0.64, 6.0], center = true);

    // Giắc đồng SMA-K cái hàn ở đầu module (Y = +18.0mm), xuyên qua thành vỏ dày 2.5mm ra ngoài
    color([0.88, 0.74, 0.22]) {
        // Đế vuông hàn vào mép bo mạch
        translate([0, 16.5, 2.5])
            cube([7.5, 4.0, 5.0], center = true);
        // Cổ ren SMA Ø6.35mm dài 11.0mm vươn từ mặt trong thành vỏ (Y = +18.0) ra ngoài (Y = +29.0)
        translate([0, 18.0, 2.5])
            rotate([-90, 0, 0]) {
                difference() {
                    cylinder(h = 11.0, r = 3.175);
                    translate([0, 0, 1.0]) cylinder(h = 11.0, r = 0.9);
                }
                // Long đen + Đai ốc lục giác SMA vặn khóa ở MẶT NGOÀI thành vỏ (sau lớp vỏ dày 2.5mm)
                translate([0, 0, day_thanh])
                    cylinder(h = 0.5, r = 5.0);
                translate([0, 0, day_thanh + 0.5])
                    cylinder(h = 2.2, r = 4.6, $fn = 6);
            }
    }
}

// 8. CẦN ANTEN SMA (RUBBER DUCKY ANTENNA)
module LinhKien_AntenSMA() {
    color([0.88, 0.74, 0.22])
        cylinder(h = 10.0, r = 3.8);
    color([0.14, 0.14, 0.16])
        translate([0, 0, 10.0])
        cylinder(h = 8.0, r = 4.6);
    color([0.10, 0.10, 0.12]) {
        translate([0, 0, 18.0])
            cylinder(h = 85.0, r1 = 4.6, r2 = 3.0);
        translate([0, 0, 103.0])
            sphere(r = 3.0);
    }
}

// 9. MẠCH HẠ ÁP MINI360 (Chuẩn 17.5 x 12.0 x 1.6 mm)
module LinhKien_Mini360() {
    color([0.10, 0.52, 0.25])
        cube([17.5, 12.0, 1.6], center = true);
    color([0.45, 0.45, 0.48])
        translate([-2.5, 0, 1.9])
        cube([5.0, 5.0, 2.2], center = true);
    color([0.75, 0.76, 0.80])
        translate([4.5, 0, 1.6])
        cube([3.8, 3.8, 1.6], center = true);
}

// 10. MẠCH SẠC PIN TP4056 CỔNG TYPE-C CÓ BẢO VỆ (Chuẩn 17.0 x 26.0 x 1.6 mm)
module LinhKien_TP4056TypeC() {
    // Bo mạch FR-4 xanh dương (17.0 x 26.0 x 1.6 mm)
    color([0.06, 0.35, 0.78])
        difference() {
            cube([17.0, 26.0, 1.6], center = true);
            // 6 Lỗ hàn dây (OUT+, B+, B-, OUT- phía dưới và IN+, IN- phía trên)
            for (dx = [-6.8, 6.8], dy = [-11.2, -6.2, 10.5])
                translate([dx, dy, 0]) cylinder(h = 3.0, r = 0.65, center = true);
        }

    // Các pad thiếc bạc quanh lỗ hàn B+/B-/OUT+/OUT-
    color([0.82, 0.84, 0.88])
        for (dx = [-6.8, 6.8], dy = [-11.2, -6.2, 10.5])
            translate([dx, dy, 0])
                difference() {
                    cube([2.6, 2.6, 1.7], center = true);
                    cylinder(h = 3.0, r = 0.65, center = true);
                }

    // Cổng cắm USB Type-C cái (9.0 x 7.4 x 3.2 mm) ở mép trên Y = +13.0mm, nhô ra +1.4mm xuyên thành vỏ
    color([0.84, 0.86, 0.90])
        translate([0, 10.7, 2.4])
        rotate([-90, 0, 0])
        linear_extrude(height = 7.4, center = true)
        hull() {
            translate([-2.9, 0]) circle(r = 1.6);
            translate([ 2.9, 0]) circle(r = 1.6);
        }
    // Lưỡi gà nhựa đen trong lòng cổng Type-C
    color([0.12, 0.12, 0.14])
        translate([0, 11.2, 2.4])
        cube([6.4, 6.0, 0.7], center = true);

    // 3 IC đen trên mặt mạch: TP4056 (SOP-8), DW01A (SOT-23-6), FS8205A (TSSOP-8)
    color([0.12, 0.12, 0.14]) {
        translate([ 0.0,  2.0, 1.55]) cube([4.2, 5.2, 1.5], center = true); // IC sạc TP4056
        translate([-4.2, -5.5, 1.35]) cube([1.8, 2.9, 1.1], center = true); // IC bảo vệ DW01A
        translate([ 2.2, -6.0, 1.40]) cube([4.4, 6.4, 1.2], center = true); // MOSFET kép 8205A
    }

    // 2 Đèn LED SMD 0603 báo trạng thái sạc nằm sát mép phải (+X = +6.5mm) chuẩn theo mạch thực tế:
    // - LED 1 (Màu Đỏ - Đang sạc / Charging) tại [6.5, +2.5]
    color([0.98, 0.15, 0.15])
        translate([6.5, 2.5, 1.25])
        cube([1.6, 1.0, 0.9], center = true);
    // - LED 2 (Màu Xanh Dương - Sạc đầy / Full) tại [6.5, -1.5]
    color([0.10, 0.65, 1.00])
        translate([6.5, -1.5, 1.25])
        cube([1.6, 1.0, 0.9], center = true);
}

// 11. NÚT NHẤN NHẢ / NHẤN GIỮ PHI 16 CÓ ĐÈN LED XANH LÁ 4 CHÂN (Chuẩn LA16 / R16-503 lỗ khoét Ø16.2mm theo đúng nút thực tế của bạn)
module LinhKien_NutNhanTrimPBS110(is_center = false) {
    // Thân trụ ren nhựa đen M16x1 dưới mặt nắp (Ø15.8mm x 15.5mm, từ Z = -15.5 đến Z = day_nap)
    color([0.12, 0.12, 0.14])
        translate([0, 0, -15.5])
        cylinder(h = 15.5 + day_nap, r = 7.9);

    // Đai ốc nhựa đen khía răng cưa M16 siết sát mặt dưới nắp (Ø19.6mm, dày 4.5mm từ Z = -4.5 đến Z = 0)
    color([0.18, 0.18, 0.20])
        translate([0, 0, -4.5])
        cylinder(h = 4.5, r = 9.8);

    // Vành viền nhựa đen (Bezel) áp sát mặt trên nắp (Ø18.8mm, cao 2.2mm từ Z = day_nap đến day_nap + 2.2)
    color([0.14, 0.14, 0.16])
        translate([0, 0, day_nap])
        cylinder(h = 2.2, r1 = 9.4, r2 = 8.6);

    // Mũ nút nhấn nhựa xuyên sáng màu Xanh Lá có đèn LED (Ø13.6mm, nhô cao 2.4mm trên vành đen)
    color([0.18, 0.78, 0.36])
        translate([0, 0, day_nap + 2.0])
        cylinder(h = 2.6, r = 6.8);

    // 4 Chân hàn mạ bạc dưới đáy nút (2 chân công tắc NO/NC + 2 chân đèn LED +/-) từ Z = -19.2 đến -15.5
    color([0.82, 0.84, 0.88]) {
        for (dx = [-4.2, 4.2], dy = [-2.6, 2.6])
            translate([dx, dy, -17.35])
            cube([0.6, 2.2, 3.7], center = true);
    }
}

// 12. CÒI BÁO BUZZER CHỦ ĐỘNG 5V (ACTIVE BUZZER Ø12.0 x 9.5 mm - NẰM NGANG DỄ HÀN)
module LinhKien_CoiBuzzer12mm() {
    // Thân trụ nhựa đen Ø12.0 x 9.5 mm nằm ngang theo trục Y
    rotate([-90, 0, 0]) {
        color([0.12, 0.12, 0.14])
            difference() {
                cylinder(h = 9.5, r = 6.0, center = true);
                // Lỗ thoát âm Ø2.6mm trên mặt trước (+Y)
                translate([0, 0, 3.5])
                    cylinder(h = 2.0, r = 1.3);
            }
        // Vòng màng rung bên trong lỗ thoát âm
        color([0.85, 0.72, 0.25])
            translate([0, 0, 3.8])
            cylinder(h = 0.3, r = 2.0);
    }
    // Dấu (+) trắng in trên đỉnh còi (Z = +6.0mm)
    color([0.95, 0.95, 0.97])
        translate([-3.25, -2.5, 6.02]) {
            cube([2.4, 0.6, 0.1], center = true);
            cube([0.6, 2.4, 0.1], center = true);
        }
    // 2 Chân hàn kim loại mạ bạc chìa ngang vào lòng hộp (-Y) tại Z = 0
    color([0.82, 0.84, 0.88]) {
        // Chân Dương (+) dài 5.8mm bên trái
        translate([-3.25, -4.75 - 2.9, 0])
            rotate([-90, 0, 0])
            cylinder(h = 5.8, r = 0.45, center = true);
        translate([-3.25, -5.0, 0])
            cube([2.0, 0.8, 2.0], center = true);
        // Chân Âm (-) dài 4.8mm bên phải
        translate([3.25, -4.75 - 2.4, 0])
            rotate([-90, 0, 0])
            cylinder(h = 4.8, r = 0.45, center = true);
        translate([3.25, -5.0, 0])
            cube([2.0, 0.8, 2.0], center = true);
    }
}

// ====================================================================
// NẮP TRÊN (TOP FACEPLATE) - TÍCH HỢP Ụ NỔI MÀN HÌNH VÁT NGHIÊNG 15°
// ====================================================================
module KhoiUNoiLCD_Ngoai() {
    // Khối ụ nổi vát nghiêng 15° đồng trục hoàn toàn với màn hình LCD:
    // Bao ngoài khoang rỗng 91.0 x 64.0 mm với thành dày 2.5mm phẳng chuẩn (96.0 x 69.0 mm), đáy gắn liền tấm nắp (Z >= 0)
    intersection() {
        // Giới hạn phía trên mặt dưới của nắp (Z >= 0) và cắt gọn vách sau đứng thẳng tại Y <= lcd_y + 35.5
        translate([lcd_x, lcd_y + 0.5, 15.0])
            cube([120.0, 70.0, 30.0], center = true);

        translate([lcd_x, lcd_y, lcd_z])
            rotate([goc_nghieng_lcd, 0, 0])
            translate([0, 0, day_nap - 14.0])
            linear_extrude(height = 28.0, center = true, convexity = 10)
            offset(r = 2.5) offset(r = -2.5)
            square([96.0, 69.0], center = true);
    }
}

module KhoiUNoiLCD_RongTrong() {
    // Khoang rỗng phẳng 100% theo mặt nghiêng 15° (z_local <= 0):
    // Kích thước lòng trong: 91.0 x 64.0 mm -> rộng hơn bo mạch LCD 87.0 x 60.0 mm đúng 2.0mm mỗi bên (4.0mm tổng)!
    // 4 trụ ốc (ngoài 86.8 x 59.8 mm) tách rời hoàn toàn khỏi 4 vách phẳng 2.1mm, cực kỳ thoáng và dễ lắp màn hình!
    translate([lcd_x, lcd_y, lcd_z])
        rotate([goc_nghieng_lcd, 0, 0])
        translate([0, 0, -15.0])
        cube([91.0, 64.0, 30.0], center = true);
}

module NapTren() {
    difference() {
        union() {
            // 1. Tấm nắp trên phẳng mịn dày 3.5mm + Khối ụ nổi màn hình vát nghiêng 15°
            difference() {
                union() {
                    KhoiKhung3D(day_nap, 0);
                    KhoiUNoiLCD_Ngoai();
                }
                // Khoét rỗng lòng ụ nổi bên dưới trần nghiêng (z_local <= 0)
                KhoiUNoiLCD_RongTrong();
            }

            // 2. 4 Trụ đỡ góc màn hình LCD 1604A (Tâm 82.0 x 55.0 mm nghiêng đồng trục 15° với trần ụ nổi)
            // Đường kính trụ Ø4.8mm (r = 2.4mm < 2.5mm lề bo mạch) -> nằm gọn 100% trong góc bo LCD!
            // Chiều cao trụ 5.0mm (từ z_local = 0 xuống z_local = -5.0mm) khớp chính xác với khung sắt LCD 7.5mm
            translate([lcd_x, lcd_y, lcd_z])
                rotate([goc_nghieng_lcd, 0, 0])
                for (dx = [-41.0, 41.0], dy = [-27.5, 27.5]) {
                    translate([dx, dy, -5.0])
                        cylinder(h = 5.8, r = 2.4);
                }

            // 3. 8 Trụ đỡ góc 2 cụm Joystick PS2 (Tâm 20.0 x 27.0 mm quanh tâm mỗi Joystick)
            // Chiều cao trụ 9.0mm (từ Z = 0 xuống Z = -9.0mm) giúp cần gạt đi hết 100% hành trình ±30° không chạm vỏ!
            for (jx = [-joy_x, joy_x]) {
                for (ox = [-10.0, 10.0], oy = [-13.5, 13.5]) {
                    translate([jx + ox, joy_y + oy, -9.0])
                        cylinder(h = 9.5, r = 2.6);
                }
            }

            // 4. 6 Trụ ren ngầm M3 đón ốc đóng nắp từ dưới đáy (nhô từ Z = 0 xuống Z = -8.0mm)
            for (pt = tru_ocs) {
                translate([pt[0], pt[1], -8.0])
                    cylinder(h = 8.5, r = 4.0); // Ø8.0mm
                // Chốt định vị đồng tâm (Ø5.6mm, dài 2.0mm) lọt vào miệng trụ dưới đáy
                translate([pt[0], pt[1], -10.0])
                    cylinder(h = 2.2, r1 = 2.6, r2 = 2.8);
            }

            // 5. Ống hướng sáng đôi (Light-Pipe Shroud) dưới mặt nắp thẳng hàng 100% với 2 LED báo sạc của mạch TP4056 (đúc liền nguyên khối vào mặt nắp)
            translate([tp_x + 6.5, tp_y + 0.5, -2.5])
                cube([5.6, 9.0, 6.0], center = true);

            // 6. Khuy móc dây đeo cổ FPV (Neck Strap Eyelet) đúc liền sống lưng giữa ngay sau gờ nghiêng LCD (X = 0, Y = +48.5)
            translate([0, 53.0, day_nap - 0.5])
                hull() {
                    translate([0, -3.5, 0.5]) cube([6.5, 4.0, 1.0], center = true);
                    translate([0,  2.5, 0.5]) cube([6.5, 4.0, 1.0], center = true);
                    translate([0,  0.0, 5.8])
                        rotate([0, 90, 0])
                        cylinder(h = 6.5, r = 4.5, center = true);
                }

            // 7. Chữ nổi 3D "Toàn Diện" màu Vàng Gold trên mặt vát nghiêng 15° dưới màn hình LCD
            color([0.96, 0.78, 0.15])
                translate([lcd_x, lcd_y, lcd_z])
                rotate([goc_nghieng_lcd, 0, 0])
                translate([0, -27.2, day_nap - 0.2])
                linear_extrude(height = 1.0)
                text("Toàn Diện", size = 6.2, font = "Arial:style=Bold", halign = "center", valign = "center");
        }

        // --- CÁC LỖ KHOÉT KỸ THUẬT CHUẨN DATASHEET ---

        // 1. Cửa sổ khung màn hình LCD 1604A (77.4 x 40.4 mm) & 4 lỗ ren mù M2.5 nghiêng đồng trục 15° trên ụ nổi
        translate([lcd_x, lcd_y, lcd_z])
            rotate([goc_nghieng_lcd, 0, 0]) {
                // Cửa sổ xuyên trần nghiêng cho khung sắt đen 77.0 x 40.0 mm
                translate([0, 0, -2.0])
                    linear_extrude(height = day_nap + 4.0, convexity = 10)
                    square([77.4, 40.4], center = true);

                // Hốc nhỏ mặt dưới trần nghiêng đón vấu đèn nền LED A/K bên phải
                translate([38.0, 0, -3.0])
                    cube([4.0, 16.5, 2.5], center = true);

                // 4 Lỗ ren mù M2.5 (Ø2.1mm, sâu 6.5mm từ z_local=-5.05 lên z_local=+1.45, chừa 2.05mm thịt trần ụ nổi đặc kín)
                for (dx = [-41.0, 41.0], dy = [-27.5, 27.5]) {
                    translate([dx, dy, -5.05])
                        cylinder(h = 6.5, r = 1.05);
                }
            }

        // 2. Hai lỗ cần Joystick PS2: Cổ lỗ Ø27.5mm kèm vát côn 45° loe lên Ø32.5mm ở mặt nắp (Full 100% hành trình)
        for (jx = [-joy_x, joy_x]) {
            translate([jx, joy_y, -2.0])
                cylinder(h = day_nap + 4.0, r = 13.75);
            translate([jx, joy_y, day_nap - 2.5])
                cylinder(h = 2.6, r1 = 13.75, r2 = 16.25);
        }

        // 3. Lỗ cổ ren M7 chiết áp xoay WH148 (Ø7.2mm chuẩn Datasheet)
        translate([pot_x, pot_y, -2.0])
            cylinder(h = day_nap + 4.0, r = 3.6);

        // 3B. 3 Lỗ cổ ren M6 cho 3 công tắc gạt MTS-102 trên 2 vai (Ø6.2mm chuẩn Datasheet)
        for (sp = sw_coords) {
            translate([sp[0], sp[1], -2.0])
                cylinder(h = day_nap + 4.0, r = 3.1);
        }

        // 4. Lỗ thân ren M16 nút nguồn kim loại (Ø16.2mm chuẩn Datasheet)
        translate([pwr_x, pwr_y, -2.0])
            cylinder(h = day_nap + 4.0, r = 8.1);

        // 4A. 3 Lỗ cổ ren M16 (Ø16.2mm) cho 3 nút nhấn phi 16 có đèn 4 chân (< Trái | MENU/OK | Phải >) ngay dưới màn hình LCD
        for (tc = trim_coords) {
            translate([tc[0], tc[1], -2.0])
                cylinder(h = day_nap + 4.0, r = 8.1);
        }

        // 4C. Lỗ ngang Ø4.8mm xuyên qua Khuy móc dây đeo cổ FPV (tại X = 0, Y = 48.5, Z = day_nap + 5.3)
        translate([0, 53.0, day_nap + 5.3])
            rotate([0, 90, 0])
            cylinder(h = 12.0, r = 2.4, center = true);

        // 4B. 2 Lỗ đèn báo sạc TP4056 xuyên mặt nắp trên (LED Đỏ Đang sạc tại Y = tp_y + 2.5, LED Xanh Đầy tại Y = tp_y - 1.5)
        //     Khoét Ø2.4mm xuyên suốt ống hướng sáng + vát phễu Ø3.6mm trên mặt nắp (vừa nhìn trực tiếp, vừa nhét vừa sợi dẫn sáng 2.2mm)
        for (ly = [tp_y + 2.5, tp_y - 1.5]) {
            translate([tp_x + 6.5, ly, -6.0])
                cylinder(h = day_nap + 8.0, r = 1.2);
            translate([tp_x + 6.5, ly, day_nap - 1.2])
                cylinder(h = 1.5, r1 = 1.2, r2 = 1.85);
        }

        // 5. 8 Lỗ ren mù M3 cho 2 bo Joystick (Ø2.7mm, sâu 10.5mm từ Z=-9.05 lên Z=+1.45, chừa 2.05mm thịt nắp đặc kín)
        for (jx = [-joy_x, joy_x]) {
            for (ox = [-10.0, 10.0], oy = [-13.5, 13.5]) {
                translate([jx + ox, joy_y + oy, -9.05])
                    cylinder(h = 10.5, r = 1.35);
            }
        }

        // 6. 6 Lỗ ren mù M3 đón ốc đóng nắp từ đáy (Ø2.7mm, sâu 11.5mm từ Z=-10.05 lên Z=+1.45)
        for (pt = tru_ocs) {
            translate([pt[0], pt[1], -10.05])
                cylinder(h = 11.5, r = 1.35);
        }

        // 7. Rãnh ngàm âm (Female Groove: từ -1.1mm đến -2.6mm) dọc mặt dưới nắp, khớp 100% với gờ ngàm dương (-1.25mm đến -2.5mm) của đáy
        translate([0, 0, -0.5])
            difference() {
                KhoiKhung3D(2.1, -1.1);
                translate([0, 0, -0.5]) KhoiKhung3D(3.5, -2.6);
            }
    }
}

// MODULE NGÀM CÀI ĐÀN HỒI (SNAP-FIT BRACKET) CHO CÁC BO MẠCH
module NgamCaiBoMach(w_pcb, d_pcb, h_seat, h_pcb, hook_axis="x", hook_w=6.0) {
    t_wall = 1.4;
    lip = 0.75;
    w_out = w_pcb + 2 * t_wall;
    d_out = d_pcb + 2 * t_wall;
    h_tot = h_seat + h_pcb + 1.2;
    z_lip = h_seat + h_pcb;

    difference() {
        translate([0, 0, h_tot / 2])
            cube([w_out, d_out, h_tot], center = true);
        translate([0, 0, h_seat + (h_tot + 2) / 2])
            cube([w_pcb, d_pcb, h_tot + 2], center = true);
        translate([0, 0, (h_seat + 0.1) / 2])
            cube([w_pcb - 3.0, d_pcb - 3.0, h_seat + 0.2], center = true);
        if (hook_axis == "x") {
            translate([0, 0, h_tot / 2]) cube([w_out + 2, d_pcb - 6.0, h_tot + 2], center = true);
            translate([0, 0, h_tot / 2]) cube([w_pcb - 6.0, d_out + 2, h_tot + 2], center = true);
        } else {
            translate([0, 0, h_tot / 2]) cube([w_pcb - 6.0, d_out + 2, h_tot + 2], center = true);
            translate([0, 0, h_tot / 2]) cube([w_out + 2, d_pcb - 6.0, h_tot + 2], center = true);
        }
    }

    if (hook_axis == "x") {
        for (sx = [-1, 1]) {
            translate([sx * (w_pcb / 2 + t_wall / 2), 0, h_tot / 2])
                cube([t_wall, hook_w, h_tot], center = true);
            translate([sx * (w_pcb / 2 - lip / 2), 0, z_lip + 0.2])
                cube([lip, hook_w, 0.4], center = true);
            translate([sx * (w_pcb / 2 - lip / 4), 0, z_lip + 0.6])
                cube([lip / 2, hook_w, 0.4], center = true);
        }
    } else {
        for (sy = [-1, 1]) {
            translate([0, sy * (d_pcb / 2 + t_wall / 2), h_tot / 2])
                cube([hook_w, t_wall, h_tot], center = true);
            translate([0, sy * (d_pcb / 2 - lip / 2), z_lip + 0.2])
                cube([hook_w, lip, 0.4], center = true);
            translate([0, sy * (d_pcb / 2 - lip / 4), z_lip + 0.6])
                cube([hook_w, lip / 2, 0.4], center = true);
        }
    }
}

// ====================================================================
// THÂN ĐÁY HỘP (BOTTOM CHASSIS) - GỌN GÀNG, CHUẨN XÁC, KHÔNG VẼ THỪA
// ====================================================================
module ThanDayHop() {
    difference() {
        union() {
            // 1. Thành vỏ ngoài dày 2.5mm và sàn đáy dày 2.5mm
            difference() {
                union() {
                    KhoiKhung3D(h_day, 0);

                    // Gờ ngàm dương (Male Lip: từ -1.25mm đến -2.5mm) cao 1.5mm dọc miệng trong của thành hộp
                    translate([0, 0, h_day - 0.5])
                        KhoiKhung3D(2.0, -1.25);
                }

                // Khoét rỗng lòng hộp (chừa sàn dày 2.5mm và thành dày 2.5mm)
                translate([0, 0, day_day])
                    KhoiKhung3D(h_day + 5.0, -day_thanh);

                // Lỗ xuyên thành trên (Y = 67.5..70.0) cho cổ ren đồng SMA-K (Ø6.6mm tại X = lora_x, Z = lora_z + 2.5 = 16.5mm)
                translate([lora_x, 76.0, lora_z + 2.5])
                    rotate([90, 0, 0])
                    cylinder(h = day_thanh + 6.0, r = 3.3, center = true);

                // Lỗ cổng sạc USB Type-C xuyên thành trên (tại X = tp_x = -26.0, Y = 67.5..70.0, Z = day_day + 5.2 = 7.7mm)
                translate([tp_x, 74.5, day_day + 5.2])
                    rotate([-90, 0, 0])
                    linear_extrude(height = 6.0, center = true)
                    hull() {
                        translate([-2.9, 0]) circle(r = 1.9);
                        translate([ 2.9, 0]) circle(r = 1.9);
                    }
                // Hốc vát lõm mặt ngoài thành vỏ (12.6 x 6.8 mm, sâu 1.0mm) để đầu nhựa cáp sạc Type-C cắm ngập sát
                translate([tp_x, 75.0, day_day + 5.2])
                    rotate([-90, 0, 0])
                    linear_extrude(height = 3.0)
                    hull() {
                        translate([-3.3, 0]) circle(r = 3.0);
                        translate([ 3.3, 0]) circle(r = 3.0);
                    }
                // 3 Khe thoát âm cho Còi báo Buzzer 12mm xuyên thành trên bên phải (tại X = buz_x +- 3.5, Y = 74.5, Z = buz_z = day_day + 8.0)
                for (bx = [buz_x - 3.5, buz_x, buz_x + 3.5]) {
                    translate([bx, 74.5, buz_z])
                        rotate([-90, 0, 0])
                        linear_extrude(height = 6.0, center = true)
                        hull() {
                            translate([0, -2.2]) circle(r = 0.9);
                            translate([0,  2.2]) circle(r = 0.9);
                        }
                }
            }

            // 2. 6 Trụ dẫn hướng ốc M3 từ sàn đáy lên Z = h_day - 8.0 (đón 6 trụ ren ngầm của nắp trên)
            for (pt = tru_ocs) {
                translate([pt[0], pt[1], day_day - 0.5])
                    cylinder(h = h_day - 8.0 - (day_day - 0.5), r = 4.0);
            }

            // 3. Khay định vị Pin 18650 + 2 Ngàm cài đàn hồi kẹp khóa đỉnh khay pin (76.0 x 40.5 mm)
            translate([bat_x, bat_y, day_day - 0.5]) {
                difference() {
                    translate([0, 0, 2.5]) cube([79.6, 44.1, 5.0], center = true);
                    translate([0, 0, 2.5]) cube([76.4, 40.9, 6.0], center = true);
                    translate([0, 0, 2.5]) cube([84.0, 22.0, 6.0], center = true);
                    translate([0, 0, 2.5]) cube([52.0, 48.0, 6.0], center = true);
                }
                // 2 Cần ngàm đàn hồi khóa mặt trên khay pin tại Z = 22.0mm
                for (sy = [-1, 1]) {
                    translate([0, sy * 20.45, 10.75])
                        cube([12.0, 1.8, 21.5], center = true);
                    translate([0, sy * 19.95, 21.5])
                        cube([12.0, 1.0, 0.8], center = true);
                }
            }

            // 4. Ngàm cài đàn hồi cho Kit STM32 Blue Pill (53.34 x 22.86 mm, 2 vấu ngàm khóa -X và +X)
            translate([stm_x, stm_y, day_day - 0.5])
                NgamCaiBoMach(53.6, 23.0, 2.5, 1.6, hook_axis="x", hook_w=7.0);

            // 5. 2 Thanh ray đỡ LoRa E22 + 2 Vấu ngàm cài đàn hồi ở đuôi (21.0 x 36.0 mm, Z = 18.0mm)
            translate([lora_x, lora_y, day_day - 0.5]) {
                for (sx = [-1, 1]) {
                    translate([sx * 11.2, 2.0, (lora_z - day_day + 2.1) / 2])
                        difference() {
                            cube([3.2, 24.0, lora_z - day_day + 2.1], center = true);
                            translate([-sx * 0.9, 0, (lora_z - day_day + 2.1) / 2 - 0.9])
                                cube([2.0, 26.0, 2.0], center = true);
                        }
                    // Vấu ngàm cài ở đuôi bo LoRa tại Y = -12.0mm
                    translate([sx * 11.2, -12.0, (lora_z - day_day + 4.7) / 2])
                        cube([2.0, 4.0, lora_z - day_day + 4.7], center = true);
                    translate([sx * 10.6, -12.0, lora_z - day_day + 2.4])
                        cube([0.8, 4.0, 0.8], center = true);
                }
            }

            // 6. Ngàm cài đàn hồi cho Mạch hạ áp Mini360 (17.5 x 12.0 mm, 2 vấu ngàm khóa -Y và +Y)
            translate([buck_x, buck_y, day_day - 0.5])
                NgamCaiBoMach(17.7, 12.2, 2.1, 1.6, hook_axis="y", hook_w=4.0);

            // 7. Ngàm cài đàn hồi cho Mạch sạc TP4056 Type-C (17.0 x 26.0 mm, 2 vấu ngàm khóa -X và +X)
            translate([tp_x, tp_y, day_day - 0.5])
                NgamCaiBoMach(17.2, 26.2, 2.5, 1.6, hook_axis="x", hook_w=7.0);

            // 8. Ngàm kẹp chữ C nằm ngang đỡ Còi Buzzer 12mm (Ø12.0mm tại [buz_x, buz_y, buz_z] = [26.5, 67.5, 10.5])
            // Nâng tâm còi lên Z = 10.5mm (cách sàn 8.0mm), hở hoàn toàn 2 chân hàn phía sau (-Y) để hàn siêu dễ!
            translate([buz_x, buz_y, day_day - 0.5]) {
                difference() {
                    union() {
                        translate([0, 0.5, 4.9]) cube([15.2, 8.0, 10.8], center = true);
                        translate([0, 5.1, 3.75])
                            difference() {
                                cube([15.2, 1.2, 8.5], center = true);
                                translate([0, -0.6, buz_z - (day_day - 0.5)])
                                    rotate([-90, 0, 0]) cylinder(h = 4.0, r = 4.5, center = true);
                            }
                    }
                    translate([0, -1.0, buz_z - (day_day - 0.5)])
                        rotate([-90, 0, 0])
                        cylinder(h = 10.0, r = 6.15, center = true);
                    translate([0, 0, 5.5])
                        cube([4.6, 12.0, 12.0], center = true);
                }
            }

            // 9. Chữ nổi 3D "Toàn Diện" màu Vàng Gold trên mặt ngoài thành trước của Thân Đáy (Y = -80.0, Z = 24.0)
            color([0.96, 0.78, 0.15])
                translate([0, -79.8, 24.0])
                rotate([90, 0, 0])
                linear_extrude(height = 1.0)
                text("Toàn Diện", size = 7.2, font = "Arial:style=Bold", halign = "center", valign = "center");
        }

        // KHOÉT 6 LỖ BẮT ỐC CHÌM TỪ MẶT ĐÁY NGƯỢC LÊN TRỤ NẮP TRÊN
        for (pt = tru_ocs) {
            // Hốc chìm âm cho mũ ốc M3 và đầu tuốc nơ vít (Ø6.4mm, từ Z = -1 lên Z = 20.0mm)
            translate([pt[0], pt[1], -1.0])
                cylinder(h = 21.0, r = 3.2);
            // Lỗ luồn thân vít M3 (Ø3.3mm xuyên suốt lên đỉnh trụ)
            translate([pt[0], pt[1], 19.0])
                cylinder(h = h_day + 2.0, r = 1.65);
            // Lỗ định vị đón chốt của trụ nắp trên (Ø5.8mm, sâu 2.2mm ở đỉnh trụ Z = h_day - 8.0)
            translate([pt[0], pt[1], h_day - 8.0 - 2.0])
                cylinder(h = 2.5, r = 2.9);
        }
    }
}

// --------------------------------------------------------------------
// ĐIỀU KHIỂN CHẾ ĐỘ HIỂN THỊ / XUẤT FILE IN 3D
// --------------------------------------------------------------------
// che_do_xem = 0 -> GỘP TẤT CẢ TRONG 1 FILE (Cụm 1: Hộp mở + 18 Linh kiện layout vào hộp | Cụm 2: Chỉ hộp lắp trên lắp dưới)
// che_do_xem = 1 -> TÁCH RỜI NẰM CẠNH NHAU (MẶC ĐỊNH: Nắp bên trái X = -135, Đáy bên phải X = +135)
// che_do_xem = 2 -> BÓC TÁCH NẮP NÂNG CAO (Exploded View: Nắp nhấc cao 60mm)
// che_do_xem = 3 -> ĐÓNG NẮP KÍN KHÍT (Assembled View)
// che_do_xem = 4 -> CHỈ HIỆN NẮP TRÊN (Xuất file STL/STEP in 3D nắp trên)
// che_do_xem = 5 -> CHỈ HIỆN ĐÁY HỘP (Xuất file STL/STEP in 3D đáy hộp)
// che_do_xem = 6 -> CHỈ HỘP LẮP TRÊN - LẮP DƯỚI (Không có linh kiện)
che_do_xem = 0;

if (che_do_xem == 4) {
    NapTren();
} else if (che_do_xem == 5) {
    ThanDayHop();
} else if (che_do_xem == 10) {
    // Nắp trên tại vị trí che_do_xem = 1 (X = -125)
    translate([-125, 0, 0]) NapTren();
} else if (che_do_xem == 11) {
    // Màn hình LCD 1604A nghiêng 15° gắn trong nắp trên
    translate([-125 + lcd_x, lcd_y, lcd_z]) rotate([goc_nghieng_lcd, 0, 0]) LinhKien_LCD1604A();
} else if (che_do_xem == 12) {
    // Joystick PS2 bên trái
    translate([-125 - joy_x, joy_y, 0]) LinhKien_JoystickPS2();
} else if (che_do_xem == 13) {
    // Joystick PS2 bên phải
    translate([-125 + joy_x, joy_y, 0]) LinhKien_JoystickPS2();
} else if (che_do_xem == 14) {
    // Chiết áp xoay WH148 + Núm vặn
    translate([-125 + pot_x, pot_y, 0]) LinhKien_ChietApWH148();
} else if (che_do_xem == 15) {
    // Nút nguồn kim loại Inox 16mm có LED Power
    translate([-125 + pwr_x, pwr_y, 0]) LinhKien_NutNguon16mm();
} else if (che_do_xem == 16) {
    // Thân đáy hộp tại vị trí che_do_xem = 1 (X = +125)
    translate([125, 0, 0]) ThanDayHop();
} else if (che_do_xem == 17) {
    // Khay 2 pin 18650 + 2 cell pin
    translate([125 + bat_x, bat_y, day_day + 0.5]) LinhKien_KhayPin18650();
} else if (che_do_xem == 18) {
    // Kit STM32F103C8T6 Blue Pill
    translate([125 + stm_x, stm_y, day_day + 2.8]) LinhKien_STM32BluePill();
} else if (che_do_xem == 19) {
    // Module RF LoRa E22 + Giắc SMA
    translate([125 + lora_x, lora_y, lora_z + 0.8]) LinhKien_LoRaE22();
} else if (che_do_xem == 20) {
    // Mạch hạ áp Mini360 Buck
    translate([125 + buck_x, buck_y, day_day + 2.4]) LinhKien_Mini360();
} else if (che_do_xem == 21) {
    // Cần Anten SMA
    translate([125 + lora_x, 76.0 + 2.7, lora_z + 2.5]) rotate([-90, 0, 0]) LinhKien_AntenSMA();

} else if (che_do_xem == 1) {
    dist_x = 135;

    // 1. NẮP TRÊN + CÁC LINH KIỆN GẮN TRÊN NẮP (ĐẶT BÊN TRÁI X = -dist_x)
    translate([-dist_x, 0, 0]) {
        color([0.22, 0.25, 0.30, 0.98])
            NapTren();

        translate([lcd_x, lcd_y, lcd_z])
            rotate([goc_nghieng_lcd, 0, 0])
            LinhKien_LCD1604A();

        for (jx = [-joy_x, joy_x]) {
            translate([jx, joy_y, 0])
                LinhKien_JoystickPS2();
        }

        translate([pot_x, pot_y, 0])
            LinhKien_ChietApWH148();

        for (sp = sw_coords) {
            translate([sp[0], sp[1], 0])
                LinhKien_CongTacMTS102();
        }

        translate([pwr_x, pwr_y, 0])
            LinhKien_NutNguon16mm();

        for (i = [0:2]) {
            translate([trim_coords[i][0], trim_coords[i][1], 0])
                LinhKien_NutNhanTrimPBS110(i == 1);
        }
    }

    // 2. THÂN ĐÁY + CÁC LINH KIỆN TRONG LÒNG ĐÁY (ĐẶT BÊN PHẢI X = +dist_x)
    translate([dist_x, 0, 0]) {
        color([0.14, 0.16, 0.18, 1.0])
            ThanDayHop();

        translate([bat_x, bat_y, day_day + 0.5])
            LinhKien_KhayPin18650();

        translate([stm_x, stm_y, day_day + 2.8])
            LinhKien_STM32BluePill();

        translate([lora_x, lora_y, lora_z + 0.8])
            LinhKien_LoRaE22();

        translate([buck_x, buck_y, day_day + 2.4])
            LinhKien_Mini360();

        translate([tp_x, tp_y, day_day + 2.8])
            LinhKien_TP4056TypeC();

        translate([buz_x, buz_y, buz_z])
            LinhKien_CoiBuzzer12mm();

        translate([lora_x, 76.0 + 2.7, lora_z + 2.5])
            rotate([-90, 0, 0])
            LinhKien_AntenSMA();
    }

} else if (che_do_xem == 2 || che_do_xem == 3) {
    z_offset = (che_do_xem == 2) ? 60 : 0;
    z_nap = h_day + z_offset;

    color([0.22, 0.25, 0.30, 0.98])
        translate([0, 0, z_nap]) NapTren();

    color([0.14, 0.16, 0.18, 1.0])
        ThanDayHop();

    translate([lcd_x, lcd_y, z_nap + lcd_z])
        rotate([goc_nghieng_lcd, 0, 0])
        LinhKien_LCD1604A();

    for (jx = [-joy_x, joy_x]) {
        translate([jx, joy_y, z_nap])
            LinhKien_JoystickPS2();
    }

    translate([pot_x, pot_y, z_nap])
        LinhKien_ChietApWH148();

    for (sp = sw_coords) {
        translate([sp[0], sp[1], z_nap])
            LinhKien_CongTacMTS102();
    }

    translate([pwr_x, pwr_y, z_nap])
        LinhKien_NutNguon16mm();

    for (i = [0:2]) {
        translate([trim_coords[i][0], trim_coords[i][1], z_nap])
            LinhKien_NutNhanTrimPBS110(i == 1);
    }

    translate([bat_x, bat_y, day_day + 0.5])
        LinhKien_KhayPin18650();

    translate([stm_x, stm_y, day_day + 2.8])
        LinhKien_STM32BluePill();

    translate([lora_x, lora_y, lora_z + 0.8])
        LinhKien_LoRaE22();

    translate([buck_x, buck_y, day_day + 2.4])
        LinhKien_Mini360();

    translate([tp_x, tp_y, day_day + 2.8])
        LinhKien_TP4056TypeC();

    translate([buz_x, buz_y, buz_z])
        LinhKien_CoiBuzzer12mm();

    translate([lora_x, 76.0 + 2.7, lora_z + 2.5])
        rotate([-90, 0, 0])
        LinhKien_AntenSMA();

} else if (che_do_xem == 6) {
    // CHỈ HỘP LẮP TRÊN - LẮP DƯỚI (ĐÓNG NẮP KÍN KHÍT, KHÔNG CÓ LINH KIỆN)
    color([0.22, 0.25, 0.30, 0.98])
        translate([0, 0, h_day]) NapTren();
    color([0.14, 0.16, 0.18, 1.0])
        ThanDayHop();

} else if (che_do_xem == 0) {
    dist_x = 135;

    // CỤM 1 (Y = 0): HỘP MỞ + TOÀN BỘ 18 LINH KIỆN RIÊNG LẺ CÓ MÀU ĐƯỢC LAYOUT VÀO HỘP
    translate([-dist_x, 0, 0]) {
        color([0.22, 0.25, 0.30, 0.98]) NapTren();
        translate([lcd_x, lcd_y, lcd_z]) rotate([goc_nghieng_lcd, 0, 0]) LinhKien_LCD1604A();
        for (jx = [-joy_x, joy_x]) translate([jx, joy_y, 0]) LinhKien_JoystickPS2();
        translate([pot_x, pot_y, 0]) LinhKien_ChietApWH148();
        for (sp = sw_coords) translate([sp[0], sp[1], 0]) LinhKien_CongTacMTS102();
        translate([pwr_x, pwr_y, 0]) LinhKien_NutNguon16mm();
        for (i = [0:2]) translate([trim_coords[i][0], trim_coords[i][1], 0]) LinhKien_NutNhanTrimPBS110(i == 1);
    }

    translate([dist_x, 0, 0]) {
        color([0.14, 0.16, 0.18, 1.0]) ThanDayHop();
        translate([bat_x, bat_y, day_day + 0.5]) LinhKien_KhayPin18650();
        translate([stm_x, stm_y, day_day + 2.8]) LinhKien_STM32BluePill();
        translate([lora_x, lora_y, lora_z + 0.8]) LinhKien_LoRaE22();
        translate([buck_x, buck_y, day_day + 2.4]) LinhKien_Mini360();
        translate([tp_x, tp_y, day_day + 2.8]) LinhKien_TP4056TypeC();
        translate([buz_x, buz_y, buz_z]) LinhKien_CoiBuzzer12mm();
        translate([lora_x, 76.0 + 2.7, lora_z + 2.5]) rotate([-90, 0, 0]) LinhKien_AntenSMA();
    }

    // CỤM 2 (X = 0, Y = +190): CHỈ HỘP LẮP TRÊN - LẮP DƯỚI (KHÔNG CÓ LINH KIỆN)
    translate([0, 190, 0]) {
        color([0.22, 0.25, 0.30, 0.98])
            translate([0, 0, h_day]) NapTren();
        color([0.14, 0.16, 0.18, 1.0])
            ThanDayHop();
    }
}
