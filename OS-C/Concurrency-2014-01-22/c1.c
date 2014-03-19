/*
 * URL: http://www.cs.unibo.it/~renzo/so/compiti/2014.01.22.tot.pdf
 * author: Tommaso Ognibene
*/

monitor bridge
{
	condition okE;    // attraversare in direzione Est
	condition okW;    // attraversare in direzione Ovest
	int n = 0;        // numero di veicoli sul ponte
	int waitingE = 0; // numero di veicoli che attendono di attraversare in direzione Est
	int waitingW = 0; // numero di veicoli che attendono di attraversare in direzione Ovest
	bool toE = true;  // direzione di attraversamento

	procedure entry enter(Vehicle vehicle)
	{
		// [Case 1] Il veicolo vuole attraversare in direzione Ovest
		if (vehicle.To == 'W')
		{
			/* Se - il numero di veicoli sul ponte ha raggiunto il massimo; oppure
			      - qualcuno sta attraversando in direzione opposta; oppure
			      - qualcuno sta attendendo di attraversare in direzione opposta */
			if (n == N || (toE && n > 0) || (!toE && waitingE > 0))
			{
				// Mi fermo e attendo di essere sbloccato
				waitingW++;
				okW.wait();
				waitingW--;
			}
			toE = false;
			n++;
			
			/* Se possibile, sblocco eventuali altri veicoli in attesa per la stessa direzione.
			 * Questo non crea starvation in quanto sono sicuramente un numero limitato. */
			if (n < N && waitingE == 0)
				okW.signal();
		}
		// [Case 2] Il veicolo vuole attraversare in direzione Est
		else
		{
			/* Se - il numero di veicoli sul ponte ha raggiunto il massimo; oppure
			      - qualcuno sta attraversando in direzione opposta; oppure
			      - qualcuno sta attendendo di attraversare in direzione opposta */
			if (n == N || (!toE && n > 0) || (toE && waitingW > 0))
			{
				// Mi fermo e attendo di essere sbloccato
				waitingE++;
				okE.wait();
				waitingE--;
			}
			toE = true;
			n++;

			/* Se possibile, sblocco eventuali altri veicoli in attesa per la stessa direzione.
			 * Questo non crea starvation in quanto sono sicuramente un numero limitato. */
			if (n < N && waitingW == 0)
				okE.signal();
		}
	}

	procedure entry exit(Vehicle vehicle)
	{
		n--;
		/* Se nessuno sta attraversando il ponte */
		if (n == 0)
		{
			// [Case 1] L'ultimo ad attraversare andava in direzione Ovest
			if (vehicle.To == 'W')
			{
				/* Se esiste, attivo il veicolo che per primo si era messo
				   in attesa per la direzione opposta */
			   if (waitingE > 0)
					okE.signal();
				else
					okW.signal();
			}
			// [Case 2] L'ultimo ad attraversare andava in direzione Est
			else
			{
				/* Se esiste, attivo il veicolo che per primo si era messo
				   in attesa per la direzione opposta */
			   if (waitingW > 0)
					okW.signal();
				else
					okE.signal();
			}
		}
	}
}
