/*
 * URL: http://www.cs.unibo.it/~renzo/so/compiti/2012-05-24.tot.pdf
 * author: Tommaso Ognibene
*/

monitor santuario
{
	condition okP;    // andare al santuario
	condition okS;    // tornare indietro
	int numP = 0;     // numero di persone sul ponte
	int numS = 0;     // numero di persone nel santuario
	int waitingP = 0; // numero di persone che attendono di andare al santuario
	int waitingS = 0; // numero di persone che attendono di tornare indietro
	bool toS = true;  // direzione di attraversamento del ponte

	procedure entry entraponte(bool goingBack)
	{
		// [Case 1] La persona vuole tornare indietro
		if (goingBack)
		{
			/* Se - il numero di persone sul ponte ha raggiunto il massimo; oppure
			      - qualcuno sta attraversando il ponte in direzione opposta; oppure
			      - qualcuno sta attendendo di attraversare nel senso opposto */
			if (numP == MAXPONTE || (toS && numP > 0) || waitingP > 0)
			{
				// Mi fermo e attendo di essere sbloccato
				waitingS++;
				okS.wait();
				waitingS--;
			}
			numP++;
			numS--;
		}
		// [Case 2] La persona vuole andare al santuario
		else
		{
			/* Se - il numero di persone sul ponte ha raggiunto il massimo; oppure
			      - il numero di persone nel santuario ha raggiunto il massimo; oppure
			      - qualcuno sta attraversando il ponte in direzione opposta; oppure
			      - qualcuno sta attendendo di attraversare nel senso opposto */
			if (numP == MAXPONTE || numP + numS == MAXSANTUARIO || (!toS && numP > 0) || waitingS > 0)
			{
				// Mi fermo e attendo di essere sbloccato
				waitingP++;
				okP.wait();
				waitingP--;
			}
			numP++;
		}
	}

	procedure entry esciponte(bool goingBack)
	{
		numP--;
		/* Se la persona ha raggiunto il santuario */
		if (!goingBack)
			numS++;
			
		/* Se nessuno sta attraversando il ponte */
		if (numP == 0)
		{
			// [Case 1] L'ultimo ad attraversare tornava indietro
			if (goingBack)
			{
				/* Se esiste, attivo la persona che per prima si era messa
				   in attesa per la direzione opposta */				
				if (waitingP > 0)
					okP.signal();
				else
					okS.signal();
			}
			// [Case 2] L'ultimo ad attraversare andava al santuario
			else
			{
				/* Se esiste, attivo la persona che per prima si era messa
				   in attesa per la direzione opposta */				
				if (waitingS > 0)
					okS.signal();
				else
					okP.signal();
			}
		}
	}
}
