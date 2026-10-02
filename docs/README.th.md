# QReader — ไทย

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [ภาษา](../README.md#qreader)

โปรแกรมอ่านรหัส QR สำหรับ Windows 7 ขึ้นไป ใช้ตัวควบคุมมาตรฐานของ Windows อ่านภาพหน้าจอ ไฟล์ภาพ และภาพในคลิปบอร์ด โดย OpenCV ค้นหาตำแหน่งและ ZXing ถอดรหัสเนื้อหา

## ดาวน์โหลดและใช้งาน

[GitHub Releases](https://github.com/halfstay/qreader/releases)

แตกไฟล์ ZIP แล้วเปิด qreader.exe โดยเก็บ libZXing.dll ไว้ในโฟลเดอร์เดียวกัน เลื่อนเมาส์ไปที่หน้าต่างหรือตัวควบคุมแล้วคลิก หรือลากเพื่อเลือกพื้นที่ กด Esc หรือคลิกขวาเพื่อยกเลิก ภาพที่จับจะถูกคัดลอกไปยังคลิปบอร์ดด้วยขนาดเดิม

| ไฟล์ | ⌨ |
| --- | --- |
| เปิดรูปภาพ — BMP / JPG / PNG | Ctrl+O |
| จับภาพหน้าจอ | F6 |
| อ่านคลิปบอร์ด | Ctrl+V |
| คัดลอกข้อความ | Ctrl+C |

## ภาษาและการตรวจจับ

การเปิดครั้งแรกใช้ภาษาของ Windows เปลี่ยนภาษาได้จากเมนูโดยไม่ต้องเริ่มโปรแกรมใหม่และผลลัพธ์ยังอยู่ มีชุดคำแปล 27 ชุดสำหรับ 26 ภาษา ครอบคลุม 209 จาก 257 เขตตาม CLDR (81.3%) โดยนับจำนวนเขต ไม่ใช่จำนวนประชากร

ภาษา → ใช้ภาษาของระบบ

สี่เหลี่ยมสีเขียวแสดงตำแหน่ง และรายการที่เลือกเป็นสีแดง ภาพจะย่อไม่เกิน 1920×1080 และค้นหารหัสเล็กเพิ่มเติมจากบริเวณพิกเซลต้นฉบับ รหัสรัศมีเฉพาะแพลตฟอร์มอาจไม่รองรับ

## การสร้าง การทดสอบ และการเผยแพร่

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
