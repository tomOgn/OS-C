/*
 * URL: http://www.cs.unibo.it/~renzo/so/compiti/2012-06-15.tot.pdf
 * author: Tommaso Ognibene
*/

monitor palindrome5
{
	// Struttura che rappresenta i numeri bloccati
	typedef struct
	{
		condition C;
		int N;
	} Stopped;
	
	Stopped stopped[10];
	for (int i = 0; i < 10; i++)
		stopped[i].N = 0;
	int n = 0;
	int palindrome[10];
	repeated = {} // Python-like dictionary
	
	procedure entry synch(int index)
	{
		// Se ho raggiunto la lunghezza del palindromo
		if (n == 10)
		{
			// Azzero le variabili
			n = 0;
			repeated = {};
		}
		// [Case 1] parte sinistra del palindromo
		if (n < 5)
		{
			// Se il numero e' gia' stato inserito
			if (repeated[index])
			{
				// Mi fermo e attendo di essere sbloccato
				stopped[index].N++;
				stopped[index].C.wait();
				stopped[index].N--;
			}
			palindrome[n++] = index;
			repeated[index] = true;
		}
		// [Case 2] parte destra del palindromo
		else
		{
			int number = palindrome[n - 5];
			/* [Case 1] Esiste (almeno) un numero in attesa che corrisponde al numero che 
			   si deve inserire. Lo riattivo. In questo modo evito la starvation. */
			if (stopped[number].N > 0)
			{
				stopped[number].C.signal();
				// Mi fermo e attendo di essere sbloccato
				stopped[index].N++;
				stopped[index].C.wait();
				stopped[index].N--;
			}
			/* [Case 1] Non esiste un numero in attesa che corrisponde al numero che 
			   si deve inserire. Inoltre l'attuale non corrisponde al numero che 
			   si deve inserire. */			
			else if (number != index)
			{
				// Mi fermo e attendo di essere sbloccato
				stopped[index].N++;
				stopped[index].C.wait();
				stopped[index].N--;				
			}
			palindrome[n++] = index;
		}
	}
}
