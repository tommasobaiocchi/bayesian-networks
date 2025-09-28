#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <cmath>
#include <cctype>

using namespace std;

const int PRECISIONE_DECIMALE = 6;

struct Fattore {
	string figlio;
	vector<string> ambito;
	unordered_map<string, int> dimensioni_dominio;
	vector<double> valori;
	vector<string> variabili;

	int calcolaIndice(const unordered_map<string, int>& assegnamento) const {
		int indice = 0;
		int moltiplicatore = 1;
		for (int i = ambito.size() - 1; i >= 0; --i) {
			const string& var = ambito[i];
			int val = assegnamento.at(var);
			indice += val * moltiplicatore;
			moltiplicatore *= dimensioni_dominio.at(var);
		}
		return indice;
	}

	double ottieniValore(const unordered_map<string, int>& assegnamento) const {
		return valori[calcolaIndice(assegnamento)];
	}

	void impostaValore(const unordered_map<string, int>& assegnamento, double valore) {
		valori[calcolaIndice(assegnamento)] = valore;
	}
};

vector<Fattore> fattori;
vector<Fattore> fattoriUtili;
vector<string> ordineEliminazione;
vector<bool> visitati;

void eliminaSpazi(string& s) {
	size_t inizio = s.find_first_not_of(" \t\r\n");
	if (inizio == string::npos) s.clear();
	else s = s.substr(inizio);
	size_t fine = s.find_last_not_of(" \t\r\n");
	if (fine != string::npos) s = s.substr(0, fine + 1);
}

bool iniziaCon(const string& s, const string& prefisso) {
	return s.size() >= prefisso.size() && s.compare(0, prefisso.size(), prefisso) == 0;
}

vector<string> dividiStringa(const string& s, char delimitatore) {
	vector<string> elementi;
	stringstream ss(s);
	string elemento;
	while (getline(ss, elemento, delimitatore)) {
		eliminaSpazi(elemento);
		if (!elemento.empty()) elementi.push_back(elemento);
	}
	return elementi;
}

string eliminaSpaziCopia(string s) {
	eliminaSpazi(s);
	return s;
}

class AnalizzatoreBIF {
public:
	unordered_map<string, int> dimensioni_dominio;
	unordered_map<string, vector<string>> domini;
	vector<Fattore> fattori;

	void analizza(const string& nomeFile) {
		ifstream file(nomeFile);
		if (!file) {
			cerr << "Impossibile aprire il file: " << nomeFile << endl;
			return;
		}

		string riga;
		while (getline(file, riga)) {
			eliminaSpazi(riga);
			if (riga.empty() || riga[0] == '%') continue;

			if (iniziaCon(riga, "variable")) {
				analizzaVariabile(file, riga);
			} else if (iniziaCon(riga, "probability")) {
				analizzaProbabilita(file, riga);
			}
		}
	}

private:
	void analizzaVariabile(ifstream& file, string& rigaIntestazione) {
		stringstream ss(rigaIntestazione);
		string tmp, nomeVariabile;
		ss >> tmp >> nomeVariabile;

		int dimensione = 0;
		vector<string> valori;
		string contenutoBlocco;

		while (getline(file, contenutoBlocco)) {
			eliminaSpazi(contenutoBlocco);
			if (contenutoBlocco.empty()) continue;

			if (contenutoBlocco.find("type discrete") != string::npos) {
				size_t parentesi1 = contenutoBlocco.find('[');
				size_t parentesi2 = contenutoBlocco.find(']');
				if (parentesi1 != string::npos && parentesi2 != string::npos) {
					string strDimensione = contenutoBlocco.substr(parentesi1 + 1, parentesi2 - parentesi1 - 1);
					dimensione = stoi(strDimensione);
				}

				size_t graffa1 = contenutoBlocco.find('{');
				size_t graffa2 = contenutoBlocco.find('}');
				if (graffa1 != string::npos && graffa2 != string::npos) {
					string strValori = contenutoBlocco.substr(graffa1 + 1, graffa2 - graffa1 - 1);
					valori = dividiStringa(strValori, ',');
				}
			}

			if (contenutoBlocco.find('}') != string::npos) break;
		}

		dimensioni_dominio[nomeVariabile] = dimensione;
		domini[nomeVariabile] = valori;
	}

	void analizzaProbabilita(ifstream& file, string& rigaIntestazione) {
		size_t inizio = rigaIntestazione.find('(');
		size_t fine = rigaIntestazione.find(')');
		if (inizio == string::npos || fine == string::npos) return;
		string contenuto = rigaIntestazione.substr(inizio + 1, fine - inizio - 1);
		string figlio;
		vector<string> genitori;

		size_t posBarra = contenuto.find('|');
		if (posBarra != string::npos) {
			figlio = contenuto.substr(0, posBarra);
			string strGenitori = contenuto.substr(posBarra + 1);
			genitori = dividiStringa(strGenitori, ',');
		} else {
			figlio = contenuto;
		}

		eliminaSpazi(figlio);
		for (string& g : genitori) eliminaSpazi(g);

		Fattore f;
		f.figlio = figlio;
		f.ambito = genitori;
		f.ambito.push_back(figlio);
		f.variabili = domini[figlio];

		for (const string& var : f.ambito) {
			f.dimensioni_dominio[var] = dimensioni_dominio[var];
		}

		int dimensioneTabella = 1;
		for (const auto& var : f.ambito) {
			dimensioneTabella *= dimensioni_dominio[var];
		}
		f.valori.resize(dimensioneTabella, 0.0);

		string riga;
		while (getline(file, riga)) {
			eliminaSpazi(riga);
			if (riga.empty()) continue;
			if (riga.find('}') != string::npos) break;

			if (iniziaCon(riga, "table")) {
				size_t inizioTabella = riga.find("table") + 5;
				size_t posPuntoVirgola = riga.find(';');
				if (posPuntoVirgola != string::npos) {
					string strTabella = riga.substr(inizioTabella, posPuntoVirgola - inizioTabella);
					vector<string> valori = dividiStringa(strTabella, ',');

					if (genitori.empty()) {
						for (size_t i = 0; i < valori.size() && i < f.valori.size(); ++i) {
							unordered_map<string, int> assegnamento;
							assegnamento[figlio] = i;
							f.impostaValore(assegnamento, stod(valori[i]));
						}
					}
				}
			} else if (riga.find('(') != string::npos) {
				size_t inizioParentesi = riga.find('(');
				size_t fineParentesi = riga.find(')');
				if (inizioParentesi != string::npos && fineParentesi != string::npos) {
					string strAssegnamento = riga.substr(inizioParentesi + 1, fineParentesi - inizioParentesi - 1);
					string strValori = riga.substr(fineParentesi + 1);

					vector<string> assegnamenti = dividiStringa(strAssegnamento, ',');
					vector<string> valori = dividiStringa(strValori, ',');

					if (assegnamenti.size() == genitori.size() && valori.size() == dimensioni_dominio[figlio]) {
						unordered_map<string, int> assegnamento;
						for (size_t i = 0; i < genitori.size(); ++i) {
							string nomeGenitore = genitori[i];
							string nomeValore = assegnamenti[i];
							eliminaSpazi(nomeValore);

							auto it = find(domini[nomeGenitore].begin(), domini[nomeGenitore].end(), nomeValore);
							if (it != domini[nomeGenitore].end()) {
								int indice = distance(domini[nomeGenitore].begin(), it);
								assegnamento[nomeGenitore] = indice;
							}
						}

						for (size_t i = 0; i < valori.size(); ++i) {
							assegnamento[figlio] = i;
							f.impostaValore(assegnamento, stod(valori[i]));
						}
					}
				}
			}
		}

		fattori.push_back(f);
	}
};

Fattore moltiplicaFattori(const Fattore& f1, const Fattore& f2) {
	Fattore risultato;

	risultato.ambito = f1.ambito;
	for (const string& var : f2.ambito) {
		if (find(risultato.ambito.begin(), risultato.ambito.end(), var) == risultato.ambito.end()) {
			risultato.ambito.push_back(var);
		}
	}

	for (const string& var : risultato.ambito) {
		if (f1.dimensioni_dominio.count(var)) {
			risultato.dimensioni_dominio[var] = f1.dimensioni_dominio.at(var);
		} else {
			risultato.dimensioni_dominio[var] = f2.dimensioni_dominio.at(var);
		}
	}

	int dimensioneTotale = 1;
	for (const auto& coppia : risultato.dimensioni_dominio) {
		dimensioneTotale *= coppia.second;
	}
	risultato.valori.resize(dimensioneTotale, 0.0);

	unordered_map<string, int> assegnamento;

	function<void(int)> ricorsione = [&](int profondita) {
		if (profondita == risultato.ambito.size()) {
			double valore = 1.0;
			unordered_map<string, int> a1, a2;
			for (const string& var : f1.ambito) a1[var] = assegnamento[var];
			for (const string& var : f2.ambito) a2[var] = assegnamento[var];
			valore = f1.ottieniValore(a1) * f2.ottieniValore(a2);
			risultato.impostaValore(assegnamento, valore);
			return;
		}

		string variabileCorrente = risultato.ambito[profondita];
		for (int i = 0; i < risultato.dimensioni_dominio[variabileCorrente]; ++i) {
			assegnamento[variabileCorrente] = i;
			ricorsione(profondita + 1);
		}
	};

	ricorsione(0);
	return risultato;
}

Fattore sommaVariabile(const Fattore& f, const string& variabile) {
	Fattore risultato;

	for (const string& var : f.ambito) {
		if (var != variabile) {
			risultato.ambito.push_back(var);
			risultato.dimensioni_dominio[var] = f.dimensioni_dominio.at(var);
		}
	}

	int dimensioneRisultato = 1;
	for (const auto& coppia : risultato.dimensioni_dominio) {
		dimensioneRisultato *= coppia.second;
	}
	risultato.valori.resize(dimensioneRisultato, 0.0);

	unordered_map<string, int> assegnamento;

	function<void(int)> ricorsione = [&](int profondita) {
		if (profondita == f.ambito.size()) {
			unordered_map<string, int> assegnamentoRidotto;
			for (const string& var : risultato.ambito) {
				assegnamentoRidotto[var] = assegnamento[var];
			}

			double valoreCorrente = f.ottieniValore(assegnamento);
			double& valoreRisultato = risultato.valori[risultato.calcolaIndice(assegnamentoRidotto)];
			valoreRisultato += valoreCorrente;
			return;
		}

		string variabileCorrente = f.ambito[profondita];
		for (int i = 0; i < f.dimensioni_dominio.at(variabileCorrente); ++i) {
			assegnamento[variabileCorrente] = i;
			ricorsione(profondita + 1);
		}
	};

	ricorsione(0);
	return risultato;
}

Fattore normalizza(const Fattore& f) {
	Fattore risultato = f;
	double somma = 0.0;

	for (double valore : risultato.valori) {
		somma += valore;
	}

	if (somma > 0.0) {
		for (double& valore : risultato.valori) {
			valore /= somma;
		}
	}

	return risultato;
}

Fattore eliminazioneVariabili(const vector<Fattore>& fattori, const vector<string>& ordineEliminazione) {
	vector<Fattore> fattoriCorrenti = fattori;

	for (const string& variabile : ordineEliminazione) {
		vector<Fattore> fattoriConVariabile;
		vector<Fattore> fattoriSenzaVariabile;

		for (const Fattore& f : fattoriCorrenti) {
			bool contieneVariabile = false;
			for (const string& var : f.ambito) {
				if (var == variabile) {
					contieneVariabile = true;
					break;
				}
			}

			if (contieneVariabile) {
				fattoriConVariabile.push_back(f);
			} else {
				fattoriSenzaVariabile.push_back(f);
			}
		}

		if (!fattoriConVariabile.empty()) {
			Fattore prodotto = fattoriConVariabile[0];
			for (size_t i = 1; i < fattoriConVariabile.size(); ++i) {
				prodotto = moltiplicaFattori(prodotto, fattoriConVariabile[i]);
			}

			Fattore sommato = sommaVariabile(prodotto, variabile);
			fattoriSenzaVariabile.push_back(sommato);
		}

		fattoriCorrenti = fattoriSenzaVariabile;
	}

	if (fattoriCorrenti.empty()) {
		Fattore vuoto;
		vuoto.valori = {1.0};
		return vuoto;
	}

	Fattore risultato = fattoriCorrenti[0];
	for (size_t i = 1; i < fattoriCorrenti.size(); ++i) {
		risultato = moltiplicaFattori(risultato, fattoriCorrenti[i]);
	}

	return normalizza(risultato);
}

void determinaOrdineEliminazione(int indice) {
	if (visitati[indice]) return;
	visitati[indice] = true;

	Fattore& fattoreCorrente = fattori[indice];

	for (size_t i = 0; i < fattoreCorrente.ambito.size() - 1; ++i) {
		string nomeGenitore = fattoreCorrente.ambito[i];

		for (size_t j = 0; j < fattori.size(); ++j) {
			if (fattori[j].figlio == nomeGenitore && !visitati[j]) {
				determinaOrdineEliminazione(j);

				if (find(ordineEliminazione.begin(), ordineEliminazione.end(), nomeGenitore) == ordineEliminazione.end()) {
					ordineEliminazione.push_back(nomeGenitore);
				}

				bool giaAggiunto = false;
				for (const Fattore& esistente : fattoriUtili) {
					if (esistente.figlio == nomeGenitore) {
						giaAggiunto = true;
						break;
					}
				}

				if (!giaAggiunto) {
					fattoriUtili.push_back(fattori[j]);
				}
			}
		}
	}
}

int main() {
	cout << "Calcolatore di Probabilità  Marginali per Reti Bayesiane" << endl;

	string nomeFile = "cancer.bif";

	AnalizzatoreBIF analizzatore;
	analizzatore.analizza(nomeFile);

	if (analizzatore.fattori.empty()) {
		cerr << "Errore: Nessun fattore trovato, controllare il file BIF." << endl;
		return 1;
	}

	fattori = analizzatore.fattori;

	cout << "\nVariabili nella rete: ";
	for (size_t i = 0; i < fattori.size(); ++i) {
		cout << fattori[i].figlio;
		if (i < fattori.size() - 1) cout << ", ";
	}
	cout << endl;

	cout << "\nPROBABILITA' MARGINALI DELLA RETE" << endl;

	for (size_t indiceTarget = 0; indiceTarget < fattori.size(); ++indiceTarget) {
		fattoriUtili.clear();
		ordineEliminazione.clear();
		visitati.assign(fattori.size(), false);

		fattoriUtili.push_back(fattori[indiceTarget]);
		determinaOrdineEliminazione(indiceTarget);

		Fattore risultato = eliminazioneVariabili(fattoriUtili, ordineEliminazione);

		cout << "\n--- " << fattori[indiceTarget].figlio << " ---" << endl;
         for (size_t j = 0; j < risultato.valori.size(); ++j) {
			cout << "P(" << fattori[indiceTarget].figlio << " = "
			     << fattori[indiceTarget].variabili[j] << ") = "
			     << fixed << setprecision(PRECISIONE_DECIMALE) << risultato.valori[j] << endl;
		}
	}
	cout << "\nCALCOLO COMPLETATO!" << endl;
	return 0;
}
