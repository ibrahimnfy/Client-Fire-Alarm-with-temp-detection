main
  Burada programın command line argumanları alabilmesini sağlıyor --name x ile clientin ismini ve --typeID x ile clientin türünü belli ediyoruz.
  Ardından Client objemizi oluşturup ip ve porta bağlama işlemini yapıyoruz.

mainwindow
  Kullanılmıyor

TestClient.cpp
  TestClient
    QElapsedTimer objesi tanımlanır(elapsedtimer) bu obje TestClient consturctorunun içinde çalışmaya başlar ve ileride sin dalgasından veri çekebilmek için kullanılacaktır.
    m_name ve m_typeID clientin isim ve tür değerlerini taşır
    socket main thread içinde tanımlanır
    Gerekli durumlarda Serialize ve Deserialize edebilmek için QProtobufSerializer objesi tanımlanır(serializer)
    Ardından gerekli sinyal slot bağlantıları kurulur.
    connect slotlarında m_timer bu cliente ait clientin veri gönderme sıklığını ifade eder
  connectAndSend 
    main.cppde çağırılır ve client socketini gerekli host ve porta bağlar
  onConnected
    Client servera connect olunca chat packageinde login message si oluşturuluyor 
    name ve typeID den sonra time set ediliyor(time cliente özel clientin mesaj göndermesi gereken kısıtlı süreyi ifade ediyor) bu süreyi server kullanıp eğer ki yeni mesaj bu sürede gelmezse clienti logout yapacak
    wrapper bizim serverea göndereceğimiz veriyi taşıyan message olacak wrapper tanımlanıp wrappera login set ediliyor
    Ardından veri serialize ediliyor ve framing yapılarak loginPackete loginLen ve loginBytes ekleniyor sırası ile. Bunun için QDataStream kullanılıyor.
    En son olarak sockete loginPacket yazılıp yayınlanıyor.
    Ardından m_timer tanımlanıyor ve başlatılıyor m_timer timeout olduğunda ontimeout üzerinden veri paylaşımı yapılmaya başlayacak.
  onReadyRead
    buffer sockete gelen yayını okuyor ve başlangıçta true initialize edilmiş onetimebufferremove içinde bufferda okunan veri 4 bytea gelene kadar if bloklarının içine giriyor.
    Buffer 4 bytetan uzun olunca bufferın ilk 4 byteı okunup (expectedsize) bufferdan siliniyor(burası proto mesajın uzunluğunu öğrenmek için var) ardından onetimebuffermove false yapılıp bu döngüden çıkılıyor.
    Ardından expectedsizea kadar buffer dolduruluyor while içinde
    Ardından bufferın expectedsize kadar byteı dataya alınıyor ve espectedsize kadar silme gerçekleşip onetimebuffermove bir sonraki okuma için true yapılıyor.
    Son olarak data wrapper üzerinden işlenebilmesi için deserialize ediliyor ve gelen veri chat ise (cliente sadece chat verisi gelmeli) veriyi gönderen ve veriye ait sensorun bilgileri konsolda yayınlanıyor.
    Aslında bu kısım bir önceki client projesinden kalmış kısım bu proje için çok da önemli değil. Bu projede clientler sadece veri yayınlasa bizim için yeterli olacaktır)
  onDisconnected
    Burada clientin host ile bağlantısı koptuğunda timer objesi hala çalışıyor ise kapatılıyor bu sayede clientin veri paylaşımı durduruluyor.(Server bağlantısı koptuğunda client boş yere çalışmasın diye)
  onTimeout
    Burası bir döngü görevi gören bir fonksiyon son kısımda m_timerın yeniden başlatılması ile her m_timer kadar zamanda başlangıçtaki connect bağlantısı ile onTimeout tekrar tekrar çalışıyor
    Ancak counter 100 olursa bu döngüden çıkılıyor (counter en sonda 1 arttırılıyor.)
    Döngüden çıkılmadığı müddetçe bir chat messagesi ile sender (clients name) ve sensor messagesi tanımlanıp v'ye sinValue ile zamana bağlı bir dalga fonksiyonundan veri çekiliyor. sensor tanımı bitince chat messageye sensor setleniyor
    Son olarak wrapper tanımlanıp chat set ediliyor. chat len başa streamlendikten sonra veri socket yardımı ile yazılıp yayınlanıyor. 
    (m_value eski projeden kalmış bir değer şu an m_value yerine v değerini kullanıyoruz)
  sinValue
    zamana bağlı (t) dalganın o anki açısını buluruz(angle)
    açının sinüs değeri (-1 ile +1 arası) verilen genlik ile çarpılıp dalganın değeri bulunur(-amp +amp arası) ve bu değer qint64e çevirilip return edilir
