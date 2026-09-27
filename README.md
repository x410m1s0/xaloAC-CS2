# xaloAC-CS2

**Educational & Research Project**

`xaloAC-CS2`, C++ ve Windows düşük seviyeli programlama konularını araştırmak amacıyla hazırlanmış deneysel bir araştırma projesidir.

Proje; user-mode / kernel-mode mimarisi, Windows driver yapısı, IOCTL iletişimi, bellek işlemleri, DirectX 11 overlay sistemi ve modüler C++ yazılım mimarisi gibi teknik konular üzerine kurulmuştur.

> **xaloAC**
> Developer: **x410m1s0**

---

## ⚠️ Proje Durumu

**Önemli:** Bu repository'deki kodların tamamı gerçek bir sistem üzerinde çalıştırılarak doğrulanmış değildir.

Kodların önemli bölümü **mantıksal akış, mimari tasarım ve kaynak kod seviyesinde ilerlenerek** oluşturulmuştur. Bileşenlerin birbirleriyle nasıl iletişim kurması gerektiği ve hedeflenen çalışma modeli tasarlanmış olsa da, bütün sistemin gerçek bir CS2 ortamında baştan sona çalıştığı garanti edilmemektedir.

Bu nedenle proje:

* Tamamlanmış bir ürün değildir.
* Production-ready değildir.
* Çalışan bir cheat olarak garanti edilmez.
* Tüm kernel ve user-mode bileşenlerinin gerçek sistem üzerinde test edildiği iddia edilmez.
* CS2'nin güncel sürümüyle uyumluluğu garanti edilmez.
* Kod içerisindeki bazı bileşenler deneysel veya tamamlanmamış olabilir.

Repository'nin amacı **çalıştığı iddia edilen hazır bir ürün sunmak değil, teknik araştırma ve geliştirme sürecini kaynak kod üzerinden paylaşmaktır.**

---

## 🎯 Projenin Amacı

`xaloAC-CS2`, aşağıdaki teknik konuları araştırmak ve öğrenmek amacıyla tasarlanmıştır:

* C++ ile düşük seviyeli Windows programlama
* User-mode ve kernel-mode mimarisi
* Windows kernel driver yapısı
* IOCTL tabanlı iletişim
* Proses ve sanal bellek işlemleri
* DirectX 11 rendering
* Overlay mimarisi
* Modüler C++ proje tasarımı
* Offset / signature tabanlı veri keşfi
* Windows sistem programlama
* Düşük seviyeli debugging ve araştırma

---

## 🧩 Mimari

Proje temel olarak üç ana katman üzerine tasarlanmıştır:

```text
┌─────────────────────────────┐
│          UserMode           │
│                             │
│ Aimbot / ESP / Triggerbot  │
│ Process / Offset / Config  │
└──────────────┬──────────────┘
               │
               │ IOCTL
               ▼
┌─────────────────────────────┐
│           Kernel            │
│                             │
│ Driver / Communication      │
│ Memory / Process Operations │
└──────────────┬──────────────┘
               │
               ▼
        Windows Kernel
```

Overlay tarafı ise görsel çıktıların oluşturulması için ayrı bir katman olarak tasarlanmıştır.

---

## 📁 Proje Yapısı

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

> Klasör yapısı proje sürümüne göre değişebilir.

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
* **Windows SDK**
* **Windows Driver Kit (WDK)**

---

## 🧠 Bileşenler

### Kernel

Kernel katmanı, Windows çekirdeğiyle çalışan düşük seviyeli bileşenlerin mimarisini içerir.

Tasarlanan bileşenler arasında:

* Driver başlangıç yapısı
* IOCTL iletişimi
* Proses işlemleri
* Bellek işlemleri
* User-mode ↔ kernel-mode iletişimi
* Düşük seviyeli sistem işlemleri

bulunmaktadır.

**Bu bileşenlerin tamamının gerçek Windows sistemi üzerinde çalıştığı doğrulanmış değildir.**

---

### UserMode

User-mode katmanı uygulamanın kullanıcı alanındaki ana mantığını içerir.

Tasarlanan bileşenler:

* ProcessManager
* OffsetScanner
* Aimbot
* ESP
* Triggerbot
* AntiDebug
* ConfigManager

şeklindedir.

Bu bileşenlerin mevcut kaynak kodları **hedeflenen mimari ve mantıksal çalışma akışına göre hazırlanmıştır.**

---

### Overlay

Overlay katmanı DirectX 11 tabanlı görsel çıktı oluşturma amacıyla tasarlanmıştır.

Kod içerisinde aşağıdaki türde görsel bileşenler hedeflenmektedir:

* Oyuncu kutuları
* İsimler
* Sağlık bilgileri
* Mesafe
* Snapline
* Head marker
* Diğer görsel bilgiler

Bunların gerçek oyun ortamındaki çalışırlığı ayrıca doğrulanmalıdır.

---

## 🔬 Geliştirme Yaklaşımı

Bu proje hazırlanırken temel yaklaşım:

```text
Mimari Tasarım
      ↓
Bileşenlerin Oluşturulması
      ↓
Modüller Arası İletişimin Tasarlanması
      ↓
Mantıksal Akışın Oluşturulması
      ↓
Kaynak Kodunun Geliştirilmesi
      ↓
Gerçek Sistem Testleri
      ↓
Hata Düzeltme
      ↓
Doğrulama
```

şeklindedir.

Ancak repository'nin mevcut halinde **tüm aşamalar tamamlanmış değildir**.

Özellikle gerçek sistem üzerinde:

* Driver yükleme,
* Kernel ↔ UserMode iletişimi,
* CS2 veri erişimi,
* Güncel offsetlerin doğrulanması,
* Overlay,
* ESP,
* Aimbot,
* Triggerbot

gibi bileşenlerin uçtan uca test edilmesi gerektiği kabul edilmektedir.

---

## 🧪 Test Durumu

Bu repository için **"tüm kodlar test edilmiştir" şeklinde bir iddia bulunmamaktadır.**

Kodların oluşturulması sırasında temel olarak:

* Kaynak kod yapısı,
* Fonksiyonlar arası mantıksal akış,
* Modüllerin birbirleriyle ilişkisi,
* Beklenen veri akışı,
* Mimari bütünlük

üzerinden ilerlenmiştir.

Gerçek donanım, gerçek Windows kernel ortamı ve güncel CS2 sürümü üzerinde yapılacak kapsamlı testler ayrı bir geliştirme aşamasıdır.

Bu nedenle herhangi bir modülün yalnızca kaynak kodda bulunması, o modülün gerçek ortamda çalıştığı anlamına gelmez.

---

## 📌 CS2 Uyumluluğu

Counter-Strike 2 güncellemeleri oyun içerisindeki veri yapılarının, offsetlerin ve diğer teknik ayrıntıların değişmesine neden olabilir.

Bu nedenle repository içerisindeki kaynak kodun herhangi bir CS2 sürümüyle sürekli olarak uyumlu kalacağı garanti edilmez.

---

## ⚠️ Kullanım ve Sorumluluk

Bu proje **eğitim ve araştırma amacıyla** yayımlanmaktadır.

Kaynak kod içerisinde oyun süreçleriyle etkileşim, kernel driver, bellek işlemleri, overlay ve benzeri düşük seviyeli teknikler bulunabilir.

Projeyi kullanan kişi, yaptığı işlemlerden ve kendi sistemindeki kullanımından kendisi sorumludur.

Çevrimiçi oyunlarda haksız avantaj sağlamak, başka kullanıcıların deneyimini bozmak veya ilgili hizmetlerin kullanım şartlarını ihlal etmek amacıyla kullanılmamalıdır.

---

## 🚫 Resmî Bağlantı Yoktur

`xaloAC-CS2`, Valve Corporation tarafından geliştirilmiş, desteklenmiş veya onaylanmış bir proje değildir.

**Counter-Strike 2** ve ilgili marka ve fikri mülkiyet hakları ilgili hak sahiplerine aittir.

Bu repository'nin Valve Corporation ile herhangi bir resmî bağlantısı olduğu iddia edilmemektedir.

---

## 📄 Lisans

Bu proje standart MIT, Apache veya GPL lisanslarından biri altında değildir.

Kullanım, değiştirme ve yeniden dağıtım koşulları için repository içerisindeki [`LICENSE`](LICENSE) dosyasına bakınız.

---

## 👤 Geliştirici

**xaloAC**

Developer:

**x410m1s0**

---

## 📌 Özet

`xaloAC-CS2`:

* Deneysel bir araştırma projesidir.
* C++ / Windows düşük seviyeli programlama üzerine kuruludur.
* User-mode ve kernel-mode mimarisi içerir.
* Kaynak kodunun tamamının gerçek ortamda test edildiği iddia edilmemektedir.
* Mimari ve mantıksal geliştirme yaklaşımıyla oluşturulmuştur.
* Production-ready değildir.
* Hazır ve garantili çalışan bir ürün olarak sunulmamaktadır.

**Amaç, kaynak kod üzerinden teknik mimariyi ve geliştirme sürecini incelemektir.**

---

**xaloAC**
**x410m1s0**
**2026**
