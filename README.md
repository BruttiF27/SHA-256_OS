# Contesto
Simulazione di una trattoria, come attori sono presenti:
    - Staff della trattoria
    - Multiple famiglie di clienti, in coda per avere un tavolo ed essere serviti.

Lo scenario presenta:
  - Giocatori (ovvero il personale): 3
  - Tavoli disponibili: 3
  - Famiglie: 9

La richiesta è di sviluppare un client in C multi-threaded che controlla uno ad uno ogni dipendente.
È fornito un server pre-compilato che modella il comportamento di qualunque cosa non sia il personale.

# Caratteristiche iniziali
Dipendenti e famiglie hanno punteggi su un insieme di parametri significativi per lo scenario, il cui comportamento deterministico è determinato da una versione coi seguenti dettagli:
    - Generata in modo pseudocasuale usando i numeri di matricola dei membri del gruppo. (VR502198, VR500802)
    - È applicata ai parametri, quindi influenza come il personale è in grado di svolgere il proprio compito e come le famiglie interagiscono con il processo di ristorazione.

# Processo di ristorazione
Workflow delle famiglie:
    1. Ingresso in trattoria e presa di un tavolo appena liberato
    2. Attesa di un aiutante che pulisca il tavolo
    3. Attesa di un cameriere per l'ordine
    4. Ordine del cibo
    5. Attesa della preparazione del cibo
    6. Attesa di consegna cibo (Dalla cucina è messo disponibile sul bancone)
    7. Consumo cibo
    8. Liberazione tavolo e attesa del turno in cassa
    9. Pagamento e abbandono trattoria

Workflow dei dipendenti per ogni famiglia:
    1. Pulizia tavolo
    2. Ricevi ordine
    3. Cucina il cibo
    4. Consegna il cibo
    5. Ricezione pagamento

Inoltre, è necessario ogni tanto che qualcuno lavi i piatti perché sono disponibili in quantità limitata, che è tuttavia sufficiente per il numero di tavoli presenti nel caso di massima quantità di cibo ordinata.

# Come giudicare la qualità del processo
Esistono solo due metriche per definire la bontà del processo di ristorazione:
    - Profitto ottenuto: Inversamente proporzionale al tempo che la famiglia passa in trattoria. Maggiore il tempo, minore il tasso di sostituzione dei clienti al tavolo, minore il profitto.
    - Reputazione: Relativa alle recensioni che le famiglie lasciano dopo aver mangiato in trattoria.

Tip: Fondamentalmente è necessario eseguire i compiti il più efficientemente possibile per massimizzare i profitti, ma bisogna mantenere una qualità sufficiente del servizio per far sì che lascino recensioni quanto più positive.

# Dettagli di funzionamento
Non tutte le informazioni sono accessibili al client; quindi è necessario osservare il comportamento restituito dal server per elaborare una risposta appropriata.

Ogni membro dello staff detiene 4 abilità e 4 tratti con valori casuali, ma la cui somma dei valori è costante. Per ognuna è possibile sapere solo se è bassa, media o alta.

Lista abilità:
    - Cameriere: Prende gli ordini e consegna il cibo
    - Aiutante: Pulisce/sparecchia i tavoli e lava i piatti
    - Cuoco: Cucina
    - Cassiere: Gestisce i pagamenti.

Lista Tratti:
    - Pazienza: Influisce quando si prendono le ordinazioni
    - Socievolezza: Influisce quando si riceve un pagamento
    - Professionalità: Influisce nelle fasi di ordinazione e cucina
    - Resistenza: Influisce su ogni ruolo svolto

Le abilità sono quindi condivise da ogni membro dello staff (ognuno può svolgere qualunque ruolo); maggiore è l'abilità, più veloce sarà il completamento del compito al quale il membro dello staff è assegnato.
I tratti di pazienza, socievolezza e professionalità influenzano la qualità del servizio e quindi la recensione finale. La resistenza invece indica quanto un membro dello staff riesce a svolgere efficacemente lo stesso ruolo per lungo tempo; ne consegue che i membri con bassa resistenza vedranno il proprio ruolo cambiato più degli altri, perché altrimenti aumenterebbe il tempo di esecuzione.

Ogni famiglia invece ha 4 pretese e 4 caratteristiche, le prime sono bilanciate come le abilità e pretese dello staff, mentre le seconde sono casuali e non bilanciati; possono quindi presentare un BST (base stat total) variabile. Salvo eccezioni, non è possibile conoscere il livello di questi aspetti.

Lista pretese:
    - Tempo di attesa: Riguarda la velocità del servizio in generale
    - Qualità del cibo: Riguarda il cibo servito
    - Qualità del servizio: Riguarda l'ordine e la consegna del cibo
    - Cortesia: Riguarda l'ordine e il pagamento

Lista Caratteristiche:
    - Velocità di ordinazione
    - Quantità di cibo ordinato
    - Velocità di consumazione: proporzionale al cibo ordinato
    - Pulizia personale: Riguarda quanto la famiglia sporca consumando. Anch'esso proporzionale al cibo ordinato.

A fine cena, le famiglie lasciano in primo luogo una recensione con un giudizio (positivo, negativo, neutro) su ognuna delle loro pretese. Ogni valutazione è oggettive ed emerge dal confronto con la qualità che avremmo con valori medi di abilità e tratti del personale allocato.
In secondo luogo, produrranno una valutazione qualitativa finale dove pesano questi giudizi secondo le proprie pretese individuali (sono quindi influenzate dalle pretese della singola istanza di famiglia)

Pro tip: La velocità a ordinare/consumare si possono vedere approssimativamente dal tempo simulato richiesto per tali fasi. Inoltre, il cibo ordinato come la pulizia personale si possono osservare qualitativamente dopo l'ordine e dopo il consumo, rispettivamente.

# Interfacciamento

- Base:
La comunicazione da client a server deve avvenire via IPC System V tramite memorie condivise e una memoria di messaggi. Devono essere usati semafori IPC System V per sincronizzare l'accesso dove necessario.

Pro tip: Una memoria condivisa implica il problema delle sezione critica. Implementando i semafori dovrebbe essere possibile fermare qualunque altro processo dall'accedervi quando non deve.

La comunicazione da client a server inizia con un messaggio di saluto dal client, al quale è corrisposto un messaggio di benvenuto con le informazioni generale necessarie lato server. Quest'ultimo invia in ordine una o più istanze e notifica il completamento di ognuna e anche la terminazione del client.

- Memoria condivisa:
  - Una prima memoria condivisa è presente in sala; ogni membro dello staff può scrivere chi deve occupare quale ruolo e chi si occupa di pulire, ordinare o servire a ogni tavolo specifico.
  - Una seconda memoria condivisa appartiene alla sala, indica qualitativamente la quantità di sporcizia e di cibo relativi a ogni tavolo. Sola lettura per il client.
  - Una terza memoria condivisa appartiene alla cucina, e indica qualitativamente la quantità di piatti puliti e sporchi presenti, il numero di ordini attivi e se il cibo di un determinato tavolo è pronto da consegnare. Sola lettura per il client.
  - Una quarta memoria condivisa espone il numero di pagamenti che devono essere effettuati perché la famiglia è in coda in cassa. Sola lettura per il client.

- Coda di messaggi
Ad ogni incremento del livello di stanchezza del personale, è inviato un messaggio in una coda comune. L'mtype (necessariamente > 0) del messaggio è pari all'ID numerico del membro del personale +1. Ciò consente a ogni thrad di estrarre solo i messaggi pertinenti. Inoltre, alla fine di ogni istanza per ogni membro del personale è stampata la stanchezza nei ruoli in cui è presente.

- Lato server
- Sala: Ogni tavolo è gestito da un processo separato dal server; appena il tavolo è libero dalla famiglia che l'ha usato, vi associa la prossima e avanza nelle fasi di pulizia, ordine cibo, consegna cibo e consumo cibo. Le prime tre fasi non avanzano senza un membro del personale che se ne occupi. Inoltre, quando una fase inizia, non può essere prelazionata.

- Cucina: Ci sono un processo server per la preparazione del cibo e un altro per il lavaggio dei piatti. Il primo prepara sequenzialmente le ordinazioni secondo FCFS, il secondo lava i piatti sporchi a blocchi del 25% dei piatti totali disponibili.

- Cassa: Singolo processo server che gestisce le famiglie in ordine di arrivo.

# Lato client - Richieste effettive
È richiesto di sviluppare due strategie per massimizzare profitto (tempo medio totale speso dalle famiglie in trattoria) e reputazione (media delle valutazioni complessive) rispettivamente; inoltre, bisogna implementare un thread distinto per ogni membro del personale. ogni filo prenderà decisioni operative (cambio di ruolo) per la strategia interagendo con il server tramite l'api fornita. Per il resto dell'implementazione si ha carta bianca.

L'interazione con il server deve, per ambo le strategie, portare al completamento della gestione di tutte le famiglie.

Il server può operare in due modalità:
  - normale: Eseguita una singola istanza di simulazione con possibilità di terminazione automatica dal client alla fine.
    - Il client nel messaggio di saluto deve specificare la strat.
    - Il server stampa gli eventi che descrivono l'andamento della simulazione dell'istanza
    - flag opzionale --speed <N> e un valore intero per velocizzare di N la simulazione.
  - verifica: Segnalata con flag --verify, esegue multiple istanze di simulazione per poter recuperare tutti i dati per verificare in modo programmatico il comportamento del client.
    - La strategia del client è ignorata
    - Flag opzionale --verbose per stampare come in modalità normale; normalmente nascosto a causa della grande quantità di istanze eseguite.

L'interfacciamento con il server segue le fasi:
  1. Client invia saluto con l'insieme delle matricole degli studenti e la strategia da imporre (se non si è in stato verifica)
  2. Il server ritorna il benvenuto con l'insieme delle informazioni qualitative sui membri del personale.
  3. Il server manda sequenzialmente le istanze da eseguire, ognuna delle quali specifica velocità, strategia usata e numero di famiglie.
  4. Un'istanza è completata quando tutte le famiglie hanno pagato e tutti i tavoli sono puliti.
  5. Al completamento di un'istanza, il server invia un messaggio di completamento con i risultati di media di tempo trascorso dalle famiglie e media di valutazioni delle famiglie.
  6. Alla terminazione dell'ultima istanza, il server invia un messaggio di fine per la terminazione del client.
