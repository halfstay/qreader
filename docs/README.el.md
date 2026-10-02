# QReader — Ελληνικά

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Γλώσσα](../README.md#qreader)

Αναγνώστης κωδικών QR για Windows 7 και νεότερα, με εγγενή στοιχεία ελέγχου Windows. Διαβάζει στιγμιότυπα οθόνης, αρχεία εικόνας και εικόνες από το πρόχειρο. Το OpenCV εντοπίζει τους κωδικούς και το ZXing αποκωδικοποιεί το περιεχόμενο.

## Λήψη και χρήση

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Αποσυμπιέστε το ZIP και εκτελέστε το qreader.exe, κρατώντας το libZXing.dll στον ίδιο φάκελο. Δείξτε ένα παράθυρο ή στοιχείο και κάντε κλικ, ή σύρετε για επιλογή περιοχής. Esc ή δεξί κλικ ακυρώνει. Η λήψη αντιγράφεται στο πρόχειρο στο αρχικό της μέγεθος.

| Αρχείο | ⌨ |
| --- | --- |
| Άνοιγμα εικόνας — BMP / JPG / PNG | Ctrl+O |
| Λήψη οθόνης | F6 |
| Ανάγνωση προχείρου | Ctrl+V |
| Αντιγραφή κειμένου | Ctrl+C |

## Γλώσσα και αναγνώριση

Στην πρώτη εκκίνηση ακολουθεί τη γλώσσα των Windows. Αλλάξτε τη από το μενού χωρίς επανεκκίνηση ή απώλεια αποτελεσμάτων. Οι 27 κατάλογοι για 26 γλώσσες καλύπτουν 209 από 257 περιοχές CLDR (81,3%), μετρώντας περιοχές και όχι πληθυσμό.

Γλώσσα → Γλώσσα συστήματος

Τα πράσινα τετράγωνα δείχνουν τις θέσεις· η επιλογή γίνεται κόκκινη. Οι εικόνες μειώνονται έως 1920×1080 και οι μικροί κωδικοί αναζητούνται επίσης στα αρχικά εικονοστοιχεία. Ιδιόκτητοι ακτινωτοί κωδικοί ενδέχεται να μην υποστηρίζονται.

## Μεταγλώττιση, δοκιμές και εκδόσεις

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
