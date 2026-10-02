# QReader — हिन्दी

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [भाषा](../README.md#qreader)

Windows 7 और बाद के संस्करणों के लिए QR कोड रीडर। Windows के मानक नियंत्रणों से स्क्रीनशॉट, छवि फ़ाइल और क्लिपबोर्ड की छवियाँ पढ़ें। OpenCV कोड का स्थान ढूँढता है और ZXing उसकी सामग्री पढ़ता है।

## डाउनलोड और उपयोग

[GitHub Releases](https://github.com/halfstay/qreader/releases)

ZIP निकालें और qreader.exe चलाएँ; libZXing.dll उसी फ़ोल्डर में रखें। विंडो या नियंत्रण पर माउस ले जाकर क्लिक करें, या क्षेत्र चुनने के लिए खींचें। Esc या दायाँ क्लिक रद्द करता है। स्क्रीनशॉट मूल आकार में क्लिपबोर्ड पर कॉपी होता है।

| फ़ाइल | ⌨ |
| --- | --- |
| चित्र खोलें — BMP / JPG / PNG | Ctrl+O |
| स्क्रीन कैप्चर करें | F6 |
| क्लिपबोर्ड पढ़ें | Ctrl+V |
| पाठ कॉपी करें | Ctrl+C |

## भाषा और पहचान

पहली बार Windows की भाषा अपनाई जाती है। मेनू से बिना पुनः आरंभ किए भाषा बदलें; परिणाम सुरक्षित रहते हैं। 26 भाषाओं के 27 अनुवाद CLDR के 257 में से 209 क्षेत्रों (81.3%) को कवर करते हैं। यह क्षेत्रों की संख्या है, जनसंख्या का अनुपात नहीं।

भाषा → सिस्टम की भाषा अपनाएँ

हरे वर्ग स्थान दिखाते हैं और चुना गया परिणाम लाल होता है। पूरी छवि अधिकतम 1920×1080 तक घटती है; छोटे कोड मूल पिक्सेल वाले क्षेत्रों में भी खोजे जाते हैं। कुछ मंचों के विशेष गोलाकार कोड समर्थित नहीं हो सकते।

## बिल्ड, परीक्षण और रिलीज़

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
