# Descrizione
Elaborato di Sistemi Operativi A.A. 2024-2025; servizio di calcolo della impronta SHA-256 di file multipli, implementato con semafori, code di messaggi e memoria condivisa su Linux.

# Cos'è SHA-256?
SHA sta per Secure Hash Algorithm e appartiene a una famiglia di algoritmi che convertono una stringa di byte di lunghezza arbitraria in un'impronta (digest) di lunghezza fissata in bit.
In questo caso parliamo di un'impronta di 256b, quindi 32B rappresentabili visivamente come una stringa di 64 simboli in HEX.

Lo scopo dell'impronta è la visualizzazione sintetica di contenuti informativi in un formato compatto. E' altamente improbabile, ma non impossibile, che due input diversi creino una stessa impronta. Viene normalmente utilizzata per indicizzazione di dati tramite hashing oppure per la verifica di integrità nei checksum.

# Requisiti
Minimo necessario:
- Implementare server che riceve richieste e invia risposte, usando code di messaggi.
- Implementare client che invia richiesta e riceve risposta, usando code di messaggi.
- Trasferire il file da client a server attraverso memoria condivisa.

Per il voto massimo:
- Istanziare processi distinti per elaborare richieste multiple concorrenti.
- Introdurre un limite al numero di processi in esecuzione, modificabile dinamicamente da un secondo client.
- Schedulare richieste precedenti in ordine di dimensione di file (SJF, deduco?).
- Usare almeno un semaforo per sincronizzare il flusso di comunicazione e/o processamento.

# Roadmap
- Corretta strutturazione del progetto mediante CMAKE
- Creazione del server
- Creazione del client
- Verifica end-to-end delle entità
- Implementazione della memoria condivisa
- Implementazione di semafori per il controllo sui blocchi della memoria condivisa
- Gestione di richieste concorrenti
