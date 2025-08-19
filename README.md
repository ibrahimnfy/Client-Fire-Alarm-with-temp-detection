# main
Burada programın command line argümanları alabilmesini sağlıyor.  
`--name x` ile client'in ismini, `--typeID x` ile client'in türünü belirtiyoruz.  
Ardından Client objemizi oluşturup IP ve porta bağlama işlemini yapıyoruz.  

# mainwindow
Kullanılmıyor.  

# TestClient.cpp

## TestClient
- `QElapsedTimer` objesi tanımlanır (`elapsedtimer`). Bu obje, TestClient constructor'ı içinde çalışmaya başlar ve ileride sinüs dalgasından veri çekebilmek için kullanılacaktır.  
- `m_name` ve `m_typeID` → client'in isim ve tür değerlerini taşır.  
- `socket` → main thread içinde tanımlanır.  
- Gerekli durumlarda serialize ve deserialize edebilmek için `QProtobufSerializer` objesi tanımlanır (`serializer`).  
- Ardından gerekli sinyal-slot bağlantıları kurulur.  
- Connect slotlarında `m_timer`, bu client’e ait verilerin gönderilme sıklığını ifade eder.  

## connectAndSend
- `main.cpp`’de çağrılır ve client socket’ini gerekli host ve porta bağlar.  

## onConnected
- Client, server’a bağlanınca chat paketinde login message oluşturulur.  
- `name` ve `typeID` set edildikten sonra `time` set edilir. (Bu süre, client’in mesaj göndermesi gereken kısıtlı süreyi ifade eder. Eğer bu sürede yeni mesaj gelmezse server client’i logout yapar.)  
- `wrapper` → server’a göndereceğimiz veriyi taşıyan message olur, wrapper tanımlanıp login wrapper’a set edilir.  
- Ardından veri serialize edilir ve framing yapılarak `loginPacket` içine `loginLen` ve `loginBytes` eklenir. Bunun için `QDataStream` kullanılır.  
- En son olarak socket’e `loginPacket` yazılıp yayınlanır.  
- Ardından `m_timer` tanımlanır ve başlatılır. Timeout olduğunda `onTimeout` üzerinden veri paylaşımı yapılmaya başlar.  

## onReadyRead
- Buffer, socket’ten gelen veriyi okur. Başlangıçta `true` initialize edilmiş `onetimebufferremove` içinde, buffer’dan okunan veri 4 byte’a gelene kadar `if` bloklarına girer.  
- Buffer 4 byte’tan uzun olduğunda ilk 4 byte okunur (`expectedSize`) ve buffer’dan silinir (bu kısım proto mesajın uzunluğunu öğrenmek için vardır). Ardından `onetimebufferremove` `false` yapılır ve döngüden çıkılır.  
- Daha sonra `expectedSize` kadar buffer doldurulur (`while` içinde).  
- `expectedSize` kadar byte, `data`ya alınır ve buffer’dan silinir. Ardından `onetimebufferremove` bir sonraki okuma için tekrar `true` yapılır.  
- Son olarak `data`, wrapper üzerinden deserialize edilerek işlenir. Gelen veri chat ise (client’e sadece chat verisi gelmelidir), veriyi gönderen ve sensör bilgileri konsolda yayınlanır.  
- Bu kısım önceki client projesinden kalmıştır; bu proje için çok önemli değildir. Bu projede client’ların sadece veri yayınlaması yeterlidir.  

## onDisconnected
- Client’in host ile bağlantısı koptuğunda, `timer` objesi hala çalışıyorsa kapatılır.  
- Böylece client’in veri paylaşımı durur. (Server bağlantısı koptuğunda client boş yere çalışmasın diye.)  

## onTimeout
- Burası döngü gibi çalışan bir fonksiyondur. En sonda `m_timer` yeniden başlatılır, böylece her timeout süresi dolduğunda `onTimeout` tekrar çalışır.  
- Ancak `counter == 100` olursa döngüden çıkılır (counter her seferinde +1 arttırılır).  
- Döngüden çıkılmadığı sürece bir chat message oluşturulur; içine sender (client’in ismi) ve sensor message set edilir. Sensor message değeri, `sinValue` fonksiyonundan (zamana bağlı dalga fonksiyonu) çekilen `v` değeridir.  
- Ardından wrapper tanımlanır, chat wrapper’a set edilir. Chat mesajının uzunluğu başa yazıldıktan sonra veri socket üzerinden gönderilir.  
- (`m_value` eski projeden kalmış bir değişkendir; artık `v` kullanıyoruz.)  

## sinValue
- Zamana bağlı (`t`) dalganın o anki açısı (`angle`) hesaplanır.  
- Açının sinüs değeri (−1 ile +1 arası), verilen genlik (`amp`) ile çarpılır → dalganın değeri bulunur (−amp ile +amp arası).  
- Bu değer `qint64` tipine çevrilir ve fonksiyon tarafından döndürülür.  
