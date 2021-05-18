# unifi-sistemi-daq

## Descrizione

Questa repository contiene la sorgente di codice per diversi programma di DAQ progetatti a lezione di "Sistemi di Acquisizione Dati" alla Università degli Studi di Firenze. Ogni cartella corrisponde a un progetto diverso (ovvero a un programma di DAQ diverso). Nell'elenco sotto, sono forniti dettagli per ognuno di questi progetti

## Elenco degli proggetti nella repository

1. **interruptions**. Un programma che prova a misurare il tempo trascorso tra due chiamate successive alla funzione gettimeofday() della libreria standard di Linux.
2. **intteruptions_and_gnuplot**. Questo programma è un'estensione di **interruptions**. L'intervallo di tempo che mette il sistema operativo tra chiamate successive alla funzione Linux gettimeofday() è disegnato su un'istogramma in Gnuplot usando una pipe Linux.
3. **labview_basics**. Diverse essercizi in labview il cui scopo e di introdurre l'uso del programma.
4. **gpib_basics**. Contiene al suo interno tre programmi elementare per interfacciarsi con uno strumento fornito da un conettore GPIB usando la libreria GPIB del Linux. Il programma "read" consente di ricevere fino a 100 bytes da uno strumento identificato dal suo indirizzo GPIB. Il programma "write" consente di mandare una stringa ASCII (ovvero un commando) a uno strumento identficato dal suo indirizzo GPIB. Il programma "chat" di communicarsi con uno strumento (come prima, identificato dal suo indirizzo GPIB) in modalità query-reply.
7. **gpib_daq**. Questo progetto è una estensione dei programmi all'interno di **gpib_basics**, adattandoli per interfacciarsi con un un'alimentatore programmabile HP6627A, che gestisce quattro lampadine a incandescenza con un filamento di tungsteno. Il programma innesca un sweep di tensione tra valore iniziali e finali forniti dall'utente, dopodiché il programma campiona i valori di tensione e corrente elettrica riportati dallo strumento ad ogni step di tensione. I dati vengono salvati in un file CSV, oppure vengono tracciati su un grafico di Gnuplot (in corrispondenza della scelta dell'utente).

La sottocartella "data" contiene anche al suo interno diverse file di dati ottenuti con sweep di tensione tra 0V e 1V a passo di 50mV,e tra 0 e 12V a passi di 500mV. Diverse macro e script per l'elaborazione di dati sono inclusi nella sottocartella "scripts", nonché un file Markdown che descrive passo a passo la procedura di analisi. I dati sono stati analisati per verificare la legge di Stefan-Boltzmann (con un eventuale contributo di dispersione termica di Fourier) e i grafici ottenuti sono inclusi nella sottocartella "results"
10. **redpitaya_scpi-lxi**. Questo programma legge una traccia da un'oscilloscopio Teledyne-Lecroy usando il protocollo lxi (Lan eXtensions for instrumentation) e l'apposite librerie in Linux.
13. **labview_redpitaya_scpi-tcp**. 
15. **redpitaya_eth-socket_and_fifo**. Questo programa si interfascia con un [Red Pitaya]{https://www.redpitaya.com/} che campiona continuamente un segnale di tensione e fa pubblici i dati attraverso ETHERNET. Il programma crea un socket TPC/IP per il servizio streamer del Red Pitaya, e legge in continuo i data frame forniti da esso, impostando un soglia che triggera in crescita del segnale. All'attivazione del trigger, il programma scrive il buffer della waveform (che consiste in 100 campioni di pre-trigger e 100 di post-trigger) in una FIFO di Linux
16. **labview_redpitaya_waveform_fifo**. Questo programma in labview apre e legge i dati che fuoriescono dalla FIFO creata dal programa **redpitaya_eth-socket_and_fifo**. Il programma calcola un baseline del segnale, cerca il massimo ed assegna la differenza di questi due valori al bin di un'istogramma.
