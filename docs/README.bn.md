# QReader — বাংলা

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [ভাষা](../README.md#qreader)

Windows 7 ও পরবর্তী সংস্করণের জন্য QR কোড রিডার। Windows-এর নিজস্ব নিয়ন্ত্রণ ব্যবহার করে স্ক্রিনশট, ছবির ফাইল ও ক্লিপবোর্ডের ছবি পড়ে। OpenCV অবস্থান শনাক্ত করে এবং ZXing বিষয়বস্তু উদ্ধার করে।

## ডাউনলোড ও ব্যবহার

[GitHub Releases](https://github.com/halfstay/qreader/releases)

ZIP খুলে qreader.exe চালান; libZXing.dll একই ফোল্ডারে রাখুন। উইন্ডো বা নিয়ন্ত্রণের ওপর মাউস রেখে ক্লিক করুন, অথবা টেনে এলাকা বাছুন। Esc বা ডান ক্লিকে বাতিল করুন। স্ক্রিনশট মূল আকারে ক্লিপবোর্ডে কপি হয়।

| ফাইল | ⌨ |
| --- | --- |
| ছবি খুলুন — BMP / JPG / PNG | Ctrl+O |
| স্ক্রিনের ছবি নিন | F6 |
| ক্লিপবোর্ড পড়ুন | Ctrl+V |
| লেখা কপি করুন | Ctrl+C |

## ভাষা ও শনাক্তকরণ

প্রথমবার Windows-এর ভাষা অনুসরণ করে। মেনু থেকে পুনরায় চালু না করেই ভাষা বদলান; ফলাফল অক্ষত থাকে। ২৬ ভাষার ২৭টি অনুবাদ CLDR-এর ২৫৭টি অঞ্চলের মধ্যে ২০৯টি (৮১.৩%) সমর্থন করে। এটি অঞ্চলের সংখ্যা, জনসংখ্যার অনুপাত নয়।

ভাষা → সিস্টেমের ভাষা অনুসরণ করুন

সবুজ বর্গ অবস্থান দেখায়; নির্বাচিত ফলাফল লাল হয়। ছবি সর্বোচ্চ 1920×1080 আকারে ছোট করা হয়; ছোট কোড মূল পিক্সেলের এলাকাতেও খোঁজা হয়। কিছু প্ল্যাটফর্মের নিজস্ব বৃত্তাকার কোড সমর্থিত নাও হতে পারে।

## বিল্ড, পরীক্ষা ও প্রকাশ

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
