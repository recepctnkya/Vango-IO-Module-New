# Hexnet IO Module — IOModule-v0-60 (aktif geliştirme)

Bu klasör **tek aktif firmware geliştirme hattıdır**. Tüm yeni özellikler, OTA ve telemetri değişiklikleri burada yapılır.

| Kavram | Değer |
|--------|--------|
| **Geliştirme hattı** | `IOModule-v0-60` (bu repo klasörü) |
| **Firmware sürümü (şu an)** | `v0.60` / CMake `0.60.0` |
| **Donanım** | ESP32-S3, `Vango-Medium` |
| **Önceki aktif hat** | `IOModule-v0-51` (`0.51.0` taban kopya) |
| **Dondurulmuş hatlar** | `IOModule-v0-30`, `IOModule-v0-40`, `IOModule-v0-50`, `IOModule-v0-51` |

## ESP-IDF CMD (Windows)

| Komut | İşlev |
|--------|--------|
| **`idf.cmd`** | IDF ortamı + proje klasörü |
| **`build.cmd`** | `idf.py build` |
| **`flash.cmd COM40`** | `idf.py -p COM40 flash` |
| **`monitor.cmd COM40`** | `idf.py -p COM40 monitor` |
| **`bfm.cmd COM40`** | build + flash + monitor |
| **`YUKLE.cmd`** | Port sorar, flash |

```bat
cd IOModule-v0-60
build.cmd
flash.cmd COM40
```

## Derleme çıktısı (platform OTA)

`build\vango_medium_v0_60.bin` — donanım + sürüm (`vango_medium` + `v0_60` ← CMake `0.60.0`).

## Telemetri `device.fw`

`hexnet_firmware_version_string()` → çalışan partition `0.60.0` veya yedek `v0.60`.

## Workspace

VS Code / Cursor: `IOModule-v0-60.code-workspace`

## Yerel ayarlar

Wi-Fi kimlik bilgilerini yerel `sdkconfig` veya cihaz kurulum portalı üzerinden ayarlayın; bu değerleri kaynak dosyalarına eklemeyin. `sdkconfig` ve makineye özel VS Code/ESP-IDF ayarları Git dışında tutulur.
