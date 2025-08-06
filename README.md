# 🧠 IoT Stock Counting System

A system for managing and recording product quantities in inventory.

## 📦 เทคโนโลยีที่ใช้

- **📡 ESP8266:** เชื่อมต่อ WiFi ส่งข้อมูลเซนเซอร์
- **🔧 PlatformIO:** สำหรับพัฒนาและอัปโหลดโค้ด
- **🔗 MQTT:** โปรโตคอลสื่อสารระหว่างอุปกรณ์
- **📊 Node-RED:** สร้าง flow การทำงานและแดชบอร์ด
- **🗄️ phpMyAdmin (MySQL):** จัดเก็บข้อมูลสินค้าในฐานข้อมูล

## ⚙️ ฟีเจอร์ของระบบ

- เริ่มสั่งนับสินค้าที่ต้องการในหน้า dashboard
- ตรวจจับจำนวนสินค้าด้วยเซ็นเซอร์แบบ real-time
- ส่งข้อมูลผ่าน MQTT ไปยัง Node-RED dashboard
- บันทึกลงฐานข้อมูล
- Dashboard แสดงข้อมูลล่าสุดแบบเรียลไทม์

## 📁 โครงสร้างไฟล์ในโปรเจกต์

| ไฟล์ / โฟลเดอร์       | รายละเอียด |
|------------------------|------------|
| `main.cpp`             | โค้ดหลักของ ESP8266 |
| `platformio.ini`       | การตั้งค่าโปรเจกต์ PlatformIO |
| `flows.json`           | Flow สำหรับ Node-RED |
| `dbnodered.sql`        | โครงสร้างฐานข้อมูล MySQL |
| `StockCountingSystem.pdf` | เอกสารนำเสนอระบบ |

## 🚀 วิธีติดตั้งและใช้งาน

1. ติดตั้ง PlatformIO และเชื่อมต่อ ESP8266
2. อัปโหลด `main.cpp` ไปยังอุปกรณ์
3. ตั้งค่า MQTT broker และนำเข้า flow ใน Node-RED จาก `flows.json`
4. สร้างฐานข้อมูลจาก `dbnodered.sql` บน phpMyAdmin
5. เปิด Node-RED dashboard เพื่อตรวจสอบข้อมูลสินค้าแบบ real-time

## 📌 จุดเด่นของระบบ

- แสดงผลข้อมูลทันที

## 📚 แหล่งข้อมูลเพิ่มเติม

- [GitHub Repository](StockCountingSystem.pdf)
- เอกสารประกอบโครงการ: `StockCountingSystem.pdf`
