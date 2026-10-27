# Descrizione
Elaborato di Sistemi Operativi A.A. 2024-2025; servizio di calcolo della impronta SHA-256 di file multipli, implementato con semafori, code di messaggi e memoria condivisa su Linux.

# Cos'è SHA-256?
SHA sta per Secure Hash Algorithm e appartiene a una famiglia di algoritmi che convertono una stringa di byte di lunghezza arbitraria in un'impronta (digest) di lunghezza fissata in bit.
In questo caso parliamo di un'impronta di 256b, quindi 32B rappresentabili visivamente come una stringa di 64 simboli in HEX.

Lo scopo dell'impronta è la visualizzazione sintetica di contenuti informativi in un formato compatto. E' altamente improbabile, ma non impossibile, che due input diversi creino una stessa impronta. Viene normalmente utilizzata per indicizzazione di dati tramite hashing oppure per la verifica di integrità nei checksum.

# Funzionalità
- Server capace di ricevere multiple richieste concorrenti e inviare risposte, usando code di messaggi.
- Client capace di inviare una richiesta e riconoscere la propria risposta, usando code di messaggi.
- Trasferimento file mediante memoria condivisa.
- Limite di processi figli del server modificabile con secondo client.
- Scheduling delle richieste in base a Shortest Job First.
- Utilizzo di semafori per garantire protezione delle sezioni di memoria condivisa.
