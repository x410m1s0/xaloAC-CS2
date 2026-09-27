# xaloAC-CS2

**Educational & Research Project**

`xaloAC-CS2`, Counter-Strike 2 üzerinde düşük seviyeli Windows programlama, user-mode / kernel-mode iletişimi, bellek erişimi, overlay oluşturma ve oyun içi veri işleme gibi konuları araştırmak amacıyla hazırlanmış bir C++ araştırma projesidir.

> **xaloAC**
> Developer: **x410m1s0**

---

## ⚠️ Sorumluluk ve Kullanım Bildirimi

Bu proje **eğitim, araştırma ve yazılım geliştirme çalışmaları** amacıyla yayımlanmaktadır.

Proje içerisinde oyun süreçleriyle etkileşim, kernel-mode driver, bellek işlemleri, overlay ve benzeri düşük seviyeli tekniklerin uygulanmasına yönelik kaynak kodları bulunabilir.

Bu kaynak kodun çevrimiçi oyunlarda haksız avantaj sağlamak, başka kullanıcıların oyun deneyimini bozmak veya ilgili oyunların kullanım şartlarını ihlal etmek amacıyla kullanılmaması gerekir.

Projeyi kullanan kişi gerçekleştirdiği tüm işlemlerden kendisi sorumludur.

**xaloAC-CS2, Valve Corporation veya Counter-Strike 2 ile bağlantılı, desteklenen veya onaylanan resmi bir proje değildir.**

---

## 🎯 Projenin Amacı

Projenin temel amacı, aşağıdaki teknik konuların pratik olarak incelenmesidir:

* C++ ile düşük seviyeli Windows programlama
* User-mode ve kernel-mode mimarisi
* Windows kernel driver yapısı
* IOCTL tabanlı iletişim
* Proses ve bellek işlemleri
* DirectX tabanlı rendering
* Overlay mimarisi
* Oyun içerisindeki verilerin işlenmesi
* Offset / signature tabanlı veri keşfi
* Modüler yazılım mimarisi
* Anti-debugging ve düşük seviyeli güvenlik araştırmaları
* Windows sistem programlama

---

## 🧩 Proje Yapısı

```text
xaloAC-CS2/
│
├── Kernel/
│   ├── AntiDetection/
│   ├── Communication/
│   ├── DriverMain/
│   ├── Hypervisor/
│   ├── MemoryOps/
│   └── PEHider/
│
├── UserMode/
│   ├── Aimbot/
│   ├── AntiDebug/
│   ├── ESPRenderer/
│   ├── OffsetScanner/
│   ├── ProcessManager/
│   └── Triggerbot/
│
├── Overlay/
│   ├── D3D11Renderer/
│   ├── DrawFunctions/
│   └── OverlayWindow/
│
├── Shared/
│
├── DriverLoader/
│
├── README.md
└── LICENSE
```

> Klasör yapısı sürüme göre değişebilir.

---

## 🔧 Kullanılan Teknolojiler

* **C++**
* **Windows API**
* **Windows Kernel API**
* **Windows Driver Development**
* **IOCTL**
* **Direct3D 11**
* **ImGui / Overlay teknolojileri**
* **Visual Studio**
* **Windows Driver Kit (WDK)**

---

## 🧠 Temel Bileşenler

### Kernel

Kernel tarafı, Windows çekirdeğinde çalışan düşük seviyeli bileşenleri içerir.

Araştırılan konular arasında:

* Driver başlangıcı
* IOCTL iletişimi
* Proses tanımlama
* Bellek işlemleri
* User-mode ↔ kernel-mode iletişimi
* Kernel seviyesinde sistem işlemleri

bulunmaktadır.

### UserMode

User-mode tarafında uygulamanın ana kontrol mantığı bulunur.

Bu bölümde:

* Proses yönetimi
* Offset tarama
* ESP
* Aimbot
* Triggerbot
* Yapılandırma
* Overlay bağlantısı

gibi bileşenler yer almaktadır.

### Overlay

Overlay katmanı, DirectX 11 tabanlı görsel çıktı ve oyun üzerine çizim yapılmasıyla ilgili bileşenleri içerir.

Örneğin:

* Oyuncu kutuları
* İsimler
* Sağlık bilgileri
* Mesafe
* Snapline
* Head marker

gibi görsel bileşenlerin oluşturulmasına yönelik kodlar bulunabilir.

---

## 📚 Eğitim Konuları

Bu proje özellikle aşağıdaki konuları öğrenmek isteyen geliştiriciler için kaynak niteliğindedir:

### User-mode / Kernel-mode

Windows uygulamalarının user-mode ve kernel-mode arasındaki çalışma modelini anlamak.

### IOCTL

Bir user-mode uygulaması ile kernel driver arasında kontrollü veri iletişiminin nasıl gerçekleştirilebileceğini incelemek.

### Bellek Yönetimi

Windows proseslerinin ve sanal belleğinin düşük seviyede nasıl ele alındığını araştırmak.

### DirectX Overlay

Direct3D 11 kullanılarak gerçek zamanlı grafik arayüzlerinin nasıl oluşturulduğunu incelemek.

### Modüler Mimari

Büyük bir C++ projesinin farklı sistemlere ayrılarak yönetilmesini incelemek.

---

## ⚠️ Proje Durumu

Bu repository **production-ready bir yazılım olarak değerlendirilmemelidir.**

Kaynak kodunda deneysel, tamamlanmamış veya sistem ortamına bağlı bileşenler bulunabilir.

Özellikle:

* Windows sürümü
* Visual Studio sürümü
* Windows Driver Kit sürümü
* Driver Signing
* CS2 güncellemeleri
* Offset değişiklikleri
* Sistem yapılandırması

çalışma durumunu etkileyebilir.

Bu nedenle repository'deki kaynak kodun belirli bir sistemde çalışacağı garanti edilmez.

---

## 🛠️ Derleme

Projeyi incelemek veya geliştirmek için genel olarak:

1. Visual Studio kurulumu
2. C++ geliştirme araçları
3. Windows SDK
4. Windows Driver Kit (WDK)
5. DirectX geliştirme bileşenleri

gereklidir.

Driver geliştirme tarafında Windows'un sürücü geliştirme ve imzalama gereksinimleri ayrıca dikkate alınmalıdır.

---

## 🔬 Araştırma Alanları

Bu proje aşağıdaki alanlarda araştırma yapmak için kullanılabilir:

```text
Windows Internals
       │
       ├── Processes
       ├── Virtual Memory
       ├── Kernel
       └── Drivers
              │
              ▼
        IOCTL Communication
              │
              ▼
          UserMode
              │
              ├── Data Processing
              ├── Overlay
              └── Configuration
```

---

## 📌 CS2 Hakkında

`xaloAC-CS2`, Counter-Strike 2'nin resmi bir bileşeni değildir.

**Counter-Strike 2** ve ilgili ticari markalar Valve Corporation'a aittir.

Bu repository'nin Valve Corporation tarafından geliştirildiği, desteklendiği veya onaylandığı iddia edilmemektedir.

---

## 📄 Lisans

Bu proje standart MIT, Apache veya GPL lisanslarından biri altında değildir.

Kullanım koşulları için repository içerisindeki [`LICENSE`](LICENSE) dosyasına bakınız.

Kaynak kodunun kullanılması, kopyalanması, değiştirilmesi veya dağıtılması LICENSE dosyasındaki şartlara tabidir.

---

## 👤 Geliştirici

**xaloAC**

Developer:

**x410m1s0**

---

## 📬 Proje

```text
Project: xaloAC-CS2
Brand:   xaloAC
Developer: x410m1s0
```

Bu repository'nin temel amacı, düşük seviyeli Windows ve C++ programlama konularında teknik araştırma ve eğitim çalışmalarına kaynak sağlamaktır.
