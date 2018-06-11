#include <iostream>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>

using namespace std;

/*Variveis globais para nomes de arquivos*/
//Arquivo Principal
char *nomeArquivoEntrada = "wow.bmp";
//HUFFMAN
char *nomeArquivoSaidaHuffman = "wowHuffman.bmp";
char *nomeArquivoCodificadoHuffman = "wow.Huffman";
//SHANNO FANO
char *nomeArquivoSaidaShannoFano = "wowShannoFano.bmp";
char *nomeArquivoCodificadoShannoFano =  "wow.ShannoFano";
//RLE
char *nomeArquivoSaidaRLE= "wowRLE.bmp";
char *nomeArquivoCodificadoRLE = "wow.RLEC";

void lerCabelhacoBMP(){
		
		ifstream entrada(nomeArquivoEntrada, ios::binary);

		BITMAPFILEHEADER header;

		entrada.read((char*) &header, sizeof(BITMAPFILEHEADER));
		cout << header.bfSize << endl;

		BITMAPINFOHEADER infoheader;

		entrada.read((char*) &infoheader, sizeof(BITMAPINFOHEADER));
		cout << infoheader.biWidth << endl;
		cout << infoheader.biHeight << endl;

		RGBQUAD rgb;
		entrada.read((char*) &rgb, sizeof(RGBQUAD));
}

void leituraByte(vector<int> &contadorByte,int &contadorLeitura ){
	
	ifstream entrada(nomeArquivoEntrada, ios::binary);

	//depurador Byte à Byte
	bool Depurador = false;

	byte Lido;

	while(! entrada.eof()){

		entrada.read((char*) &Lido, sizeof(byte));
		contadorLeitura++;
		contadorByte.at(Lido) += 1;

		if(Depurador){
			cout << Lido << endl;
			system("pause");
		}

	}
	entrada.close();
}

struct Data{
	list<int> indice;
	float freq;
	Data *DescendenteEsquerdo;
	Data *DescendenteDireito;
};

bool comparaData(Data* A,Data* B){
	if(A->freq < B->freq)
		return true;
	return false;
}

bool comparaDataShano(Data* A,Data* B){
	if(B->freq < A->freq)
		return true;
	return false;
}

void gerarArvoreHuffman(list<Data*> &vetorF1){

	//Arvore de Huffman
	while(vetorF1.size() > 1){
		vetorF1.sort(comparaData);

		Data *alfa = vetorF1.front();
		vetorF1.pop_front();
		Data *beta = vetorF1.front();
		vetorF1.pop_front();

		Data *NovoCaracter = new Data;
		NovoCaracter->freq = alfa->freq + beta->freq;

		for(list<int>::iterator iter = alfa->indice.begin(); iter != alfa->indice.end(); iter++)
			NovoCaracter->indice.push_back(*iter);

		for(list<int>::iterator iter = beta->indice.begin(); iter != beta->indice.end(); iter++)
			NovoCaracter->indice.push_back(*iter);

		NovoCaracter->DescendenteEsquerdo = alfa;
		NovoCaracter->DescendenteDireito  = beta;

		vetorF1.push_back(NovoCaracter);		
	}
}

vector<string> obterCodigosCaracter(list<Data*> vetorF1){
	//Geração de codigos para cada caracter
	Data * Dado =  vetorF1.front();
	vector<string> Codigos;

	for(int indiceCaracter = 0; indiceCaracter < 256; indiceCaracter++){
		string codigo = "";
		Data * Dado =  vetorF1.front();
		Dado->indice.sort();

		while(Dado->indice.size() != 1){
	
			Dado->DescendenteDireito->indice.sort();
			Dado->DescendenteEsquerdo->indice.sort();

			boolean checkPoints = false;
			for(list<int>::iterator iter = Dado->DescendenteDireito->indice.begin(); iter != Dado->DescendenteDireito->indice.end(); iter++)
				if( (*iter) == indiceCaracter ){
					codigo += "0";
					Dado = Dado->DescendenteDireito;
					checkPoints =  true;
					break;
				}
			if(! checkPoints ){
				codigo += "1";
				Dado = Dado->DescendenteEsquerdo;
			}

		}
		
		Codigos.push_back(codigo);
		
	}
	return Codigos;
}

void compactarHuffman(list<Data*> vetorF1){

	//Geração Arquivo Saida
	ifstream entrada(nomeArquivoEntrada, ios::binary);
	ofstream saida(nomeArquivoCodificadoHuffman, ios::binary);

	//Gravar Frequencias
	for(int i = 0; i < 256; i++){
		saida.write((char*) &vetorF1.front()->freq, sizeof(float));
		vetorF1.push_back(vetorF1.front());
		vetorF1.pop_front();
	}
	
	gerarArvoreHuffman(vetorF1);

	vector<string> Codigos = obterCodigosCaracter(vetorF1);

	//depurador Byte à Byte
	bool Depurador = false;
	
	byte CaracterAtual;
	int buffer = 0;
	int qtdBitsBuffer = 0;
	while(!entrada.eof()){
		entrada.read((char*) &CaracterAtual, sizeof(byte));
		for(unsigned int i  = 0; i < Codigos.at(CaracterAtual).size() ; i++){
			if(qtdBitsBuffer == 8){
				saida.write((char*) &buffer, sizeof(byte));
				qtdBitsBuffer = 0;
				buffer = 0;
			}
			if(Codigos.at(CaracterAtual).at(i) == '1')
				buffer += 1;
			qtdBitsBuffer++;
			if(qtdBitsBuffer < 8)
				buffer = buffer << 1;		
		}
		if(Depurador){
			cout << CaracterAtual << endl;
			system("pause");
		}
	}

	if(qtdBitsBuffer != 0){ 
		while(qtdBitsBuffer < 7){
			buffer = buffer << 1;
			qtdBitsBuffer++;
		}	
		saida.write((char*) &buffer, sizeof(byte));
	}
	entrada.close();
	saida.close();
}

list<Data*> histograma(vector<int> contadorByte,int contadorLeitura){
	
	list<Data*> vetorF1;
	
	for(int i = 0; i < 256; i++){
		float resultado = float(contadorByte.at(i))/contadorLeitura;

		Data *Dados = new Data();
		Dados->indice.push_back(i);
		Dados->freq = resultado;
		
		vetorF1.push_back(Dados);
	}	
	return vetorF1;
}

void DescompactarHuffman(){

	ifstream entrada2(nomeArquivoCodificadoHuffman, ios::binary);
	ofstream saida2(nomeArquivoSaidaHuffman, ios::binary);

	list<Data*> vetorF1;
	float freq;
	for(int i = 0; i < 256; i++){
		
		entrada2.read((char*) &freq, sizeof(float));
		Data *FreqDescompactar = new Data();
		FreqDescompactar->indice.push_back(i);
		FreqDescompactar->freq = freq;
		vetorF1.push_back(FreqDescompactar);
	}

	gerarArvoreHuffman(vetorF1);

	//depurador Byte à Byte
	bool Depurador = false;
	
	byte CaracterAtual;
	int indiceCabeçaLeitura = 0;

	Data *Dado =  vetorF1.front();
	while(!entrada2.eof()){

		bool PrecisoDeBuffer = true;
		bool DaParaLer = true;
		indiceCabeçaLeitura = 0;
		while(Dado->indice.size() != 1 && DaParaLer){

			if(PrecisoDeBuffer){
				if(entrada2.good()){
					entrada2.read((char*) &CaracterAtual, sizeof(byte));
					PrecisoDeBuffer = false;
				}
				else{
					DaParaLer = false;
					break;
				}
			}
			int byteLido = (int)CaracterAtual;
			vector<char> palavraBuffer(8);

			for(int i = 0; i < 8; i++){
				if(byteLido%2 == 0)
					palavraBuffer.at(7-i) = '0';
				else
					palavraBuffer.at(7-i) = '1';
				byteLido = byteLido >> 1;
			}
		
			for( ;indiceCabeçaLeitura < 8 && Dado->indice.size() != 1; indiceCabeçaLeitura++)
				if(palavraBuffer.at(indiceCabeçaLeitura) == '0')
					Dado = Dado->DescendenteDireito;
				else
					Dado = Dado->DescendenteEsquerdo;

			if( Dado->indice.size() == 1 ){
 				saida2.write((char*) &Dado->indice.front(), sizeof(byte));
				//cout << Dado->indice.front() << endl;
				//system("pause");
				Dado = vetorF1.front();
				if(indiceCabeçaLeitura == 8){
					PrecisoDeBuffer = true;
					indiceCabeçaLeitura = 0;
				}
			}
			else{
				if(indiceCabeçaLeitura == 8){
					PrecisoDeBuffer = true;
					indiceCabeçaLeitura = 0;
				}
			}
			if(Depurador){
				for(int i = 0; i < 8; i++)
					cout << palavraBuffer.at(i);
				cout << endl;
			}
		}
	}

	entrada2.close();
	saida2.close();
}

void HuffmanC(){

	cout << "Compactando utilizando Huffman\n";
	int contadorLeitura = 0;
	vector<int> contadorByte(256,0);
	
	leituraByte(contadorByte,contadorLeitura);
	list<Data*> v = histograma(contadorByte,contadorLeitura);
	compactarHuffman(v);
	cout << "Compactando utilizando Huffman Finalizada\n";
	
}

void HuffmanD(){

	cout << "Iniciando Processo de descompactacao de Huffman\n";
	DescompactarHuffman();
	cout << "Descompactacao por Huffman concluida\n";
}

void RLEC(){
	
	ifstream entrada(nomeArquivoEntrada, ios::binary);

	ofstream saida(nomeArquivoCodificadoRLE, ios::binary);

	//depurador Byte à Byte
	bool Depurador = false;
	
	int contadorCaracter = 1;
	byte byteAtualBuffer;
	byte Lido;

	entrada.read((char*) &byteAtualBuffer, sizeof(byte));

	while(!entrada.eof()){

		entrada.read((char*) &Lido, sizeof(byte));

		if(Lido == byteAtualBuffer)
			contadorCaracter++;
		else{
			saida.write((char*) &contadorCaracter, sizeof(byte));
			saida.write((char*) &byteAtualBuffer, sizeof(byte));
			byteAtualBuffer = Lido;

			contadorCaracter = 1;
		}

		if(Depurador){
			cout << Lido << endl;
			system("pause");
		}
	}
	if(contadorCaracter != 1){
		saida.write((char*) &contadorCaracter, sizeof(byte));
		saida.write((char*) &byteAtualBuffer, sizeof(byte));
	}

	entrada.close();
	saida.close();
}

void RLED(){
	ifstream entrada(nomeArquivoCodificadoRLE, ios::binary);
	ofstream saida(nomeArquivoSaidaRLE, ios::binary);

	//depurador Byte à Byte
	bool Depurador = false;
	
	byte Contador;

	byte CaracterAtual;

	while(!entrada.eof()){

		entrada.read((char*) &Contador, sizeof(byte));

		entrada.read((char*) &CaracterAtual, sizeof(byte));

		for(int i = 0; i < int(Contador); i++){
			saida.write((char*) &CaracterAtual, sizeof(byte));
		}
		

		if(Depurador){
			cout << CaracterAtual << endl;
			system("pause");
		}
	}
	entrada.close();
	saida.close();
}

void comparaArquivos(){
	ifstream entrada(nomeArquivoEntrada, ios::binary);

	ifstream entrada2(nomeArquivoSaidaShannoFano, ios::binary);

	//depurador Byte à Byte
	bool Depurador = false;
	
	byte original;
	byte Piratex;
	
	while(!entrada.eof()){
		
		entrada.read((char*) &original, sizeof(byte));
		entrada2.read((char*) &Piratex, sizeof(byte));
		
		if(original != Piratex){
			cout << "Error: Original: " << original << " Piratex: "  << Piratex << endl;
			system("pause");
			
		}
		if(Depurador){
			cout << "Original: " << original << " Piratex: "  << Piratex << endl;
			system("pause");
		}
	}
	entrada.close();
	entrada2.close();
}

Data * gerarArvoreShannoFano(list<Data*> &vetorF1){

	vetorF1.sort(comparaDataShano);

	vector<float> freq(256,0);
	Data *NovoCaracter = new Data;

	NovoCaracter->freq = 1;

	for(int i = 0;i < 256; i++){
		int indice = vetorF1.front()->indice.front();
		freq.at(indice) = vetorF1.front()->freq;
		NovoCaracter->indice.push_back(indice);

		vetorF1.push_back(vetorF1.front());
		vetorF1.pop_front();		
	}

	//Arvore de Shannon Fano
	queue<Data*> Arvore;
	Data *ArvoreInicio = NovoCaracter;
	Arvore.push(NovoCaracter);	
	while(Arvore.size() > 0){
		NovoCaracter = Arvore.front();
		if(NovoCaracter->indice.size() > 1){
			Data *Alfa = new Data;
			Alfa->freq = 0;
			list<int>::iterator it = NovoCaracter->indice.begin();
			while(Alfa->freq < (NovoCaracter->freq)/2){
				int indiceAtual = (*it);
				Alfa->freq += freq.at(indiceAtual);
				Alfa->indice.push_back((*it));
				it++;
			}
		
	
			Data *beta = new Data;
			beta->freq = NovoCaracter->freq - Alfa->freq;
			while(it != NovoCaracter->indice.end()){
				beta->indice.push_back((*it));
				it++;
			}

			NovoCaracter->DescendenteEsquerdo = Alfa;
			NovoCaracter->DescendenteDireito  = beta;

			Arvore.push(Alfa);
			Arvore.push(beta);
		}
		Arvore.pop();
	}

	return ArvoreInicio;
}

vector<string> obterCodigosCaracterS(Data *ArvoreInicio){

	//Geração de codigos para cada caracter
	vector<string> Codigos;

	for(int indiceCaracter = 0; indiceCaracter < 256; indiceCaracter++){
		string codigo = "";
		Data *Dado = ArvoreInicio;

		Dado->indice.sort();

		while(Dado->indice.size() != 1){
	
			Dado->DescendenteDireito->indice.sort();
			Dado->DescendenteEsquerdo->indice.sort();

			boolean checkPoints = false;
			for(list<int>::iterator iter = Dado->DescendenteDireito->indice.begin(); iter != Dado->DescendenteDireito->indice.end(); iter++)
				if( (*iter) == indiceCaracter ){
					codigo += "0";
					Dado = Dado->DescendenteDireito;
					checkPoints =  true;
					break;
				}
			if(! checkPoints ){
				codigo += "1";
				Dado = Dado->DescendenteEsquerdo;
			}

		}
		
		Codigos.push_back(codigo);
		
	}
	return Codigos;
}

void compactarShannoFano(list<Data*> vetorF1, vector<string> Codigos,vector<float> freq){
	
	//Geração Arquivo Saida
	ifstream entrada(nomeArquivoEntrada, ios::binary);
	ofstream saida(nomeArquivoCodificadoShannoFano, ios::binary);

	//Gravar Frequencias
	for(int i = 0; i < 256; i++){
		saida.write((char*) &freq.at(i), sizeof(float));
	}

	//depurador Byte à Byte
	bool Depurador = false;
	
	byte CaracterAtual;
	int buffer = 0;
	int qtdBitsBuffer = 0;
	while(!entrada.eof()){

		entrada.read((char*) &CaracterAtual, sizeof(byte));

		for(unsigned int i  = 0; i < Codigos.at(CaracterAtual).size() ; i++){

			if(qtdBitsBuffer == 8){
				saida.write((char*) &buffer, sizeof(byte));
				qtdBitsBuffer = 0;
				buffer = 0;
			}	

			if(Codigos.at(CaracterAtual).at(i) == '1')
				buffer += 1;
			qtdBitsBuffer++;
			if(qtdBitsBuffer < 8)
				buffer = buffer << 1;		
		}	

		if(Depurador){
			cout << CaracterAtual << endl;
			system("pause");
		}
	}

	if(qtdBitsBuffer != 0){ 
		while(qtdBitsBuffer < 7){
			buffer = buffer << 1;
			qtdBitsBuffer++;
		}	
		saida.write((char*) &buffer, sizeof(byte));
	}
	entrada.close();
	saida.close();
	
}

void ShannoFanoC(){

	cout << "Compactando utilizando Shanno Fano\n";
	int contadorLeitura = 0;
	vector<int> contadorByte(256,0);
	leituraByte(contadorByte,contadorLeitura);
	list<Data*> v = histograma(contadorByte,contadorLeitura);


	vector<float> frequencias(256,0);
	//Gravar Frequencias
	for(int i = 0; i < 256; i++){
		frequencias.at(v.front()->indice.front())= v.front()->freq;
		v.push_back(v.front());
		v.pop_front();		
	}

	Data * ArvoreInicio = gerarArvoreShannoFano(v);

	vector<string> Codigos = obterCodigosCaracterS(ArvoreInicio);
	
	compactarShannoFano(v,Codigos,frequencias);
	cout << "Compactando utilizando Shanno Fano Finalizada\n";
	
}

void descompactarShannoFano(){

	//Descompactar
	ifstream entrada2(nomeArquivoCodificadoShannoFano, ios::binary);
	ofstream saida2(nomeArquivoSaidaShannoFano, ios::binary);

	cout << "Iniciando Descompactacao\n";

	//depurador Byte à Byte
	bool Depurador = false;

	list<Data*> vetorF1;

	float freq;
	for(int i = 0; i < 256; i++){
		
		entrada2.read((char*) &freq, sizeof(float));
		Data *FreqDescompactar = new Data();
		FreqDescompactar->indice.push_back(i);
		FreqDescompactar->freq = freq;
		vetorF1.push_back(FreqDescompactar);
		//cout << freq << endl;
	}


	Data * ArvoreInicio = gerarArvoreShannoFano(vetorF1);
	
	Data *Dado =  ArvoreInicio;
	byte CaracterAtual;
	int indiceCabeçaLeitura = 0;

	while(!entrada2.eof()){

		bool PrecisoDeBuffer = true;
		bool DaParaLer = true;
		indiceCabeçaLeitura = 0;
		while(Dado->indice.size() != 1 && DaParaLer){

			if(PrecisoDeBuffer){
				if(entrada2.good()){
					entrada2.read((char*) &CaracterAtual, sizeof(byte));
					PrecisoDeBuffer = false;
				}
				else{
					DaParaLer = false;
					break;
				}
			}
			int byteLido = (int)CaracterAtual;
			vector<char> palavraBuffer(8);

			for(int i = 0; i < 8; i++){
				if(byteLido%2 == 0)
					palavraBuffer.at(7-i) = '0';
				else
					palavraBuffer.at(7-i) = '1';
				byteLido = byteLido >> 1;
			}
		
			for( ;indiceCabeçaLeitura < 8 && Dado->indice.size() != 1; indiceCabeçaLeitura++)
				if(palavraBuffer.at(indiceCabeçaLeitura) == '0')
					Dado = Dado->DescendenteDireito;
				else
					Dado = Dado->DescendenteEsquerdo;

			if( Dado->indice.size() == 1 ){
 				saida2.write((char*) &Dado->indice.front(), sizeof(byte));
				//cout << (char)Dado->indice.front() << endl;
				//system("pause");
				Dado = ArvoreInicio;
				if(indiceCabeçaLeitura == 8){
					PrecisoDeBuffer = true;
					indiceCabeçaLeitura = 0;
				}
			}
			else{
				if(indiceCabeçaLeitura == 8){
					PrecisoDeBuffer = true;
					indiceCabeçaLeitura = 0;
				}
			}
			if(Depurador){
				for(int i = 0; i < 8; i++)
					cout << palavraBuffer.at(i);
				cout << endl;
			}
		}
	}

	entrada2.close();
	saida2.close();
	
}

void ShannoFanoD(){
	cout << "Iniciando Processo de descompactacao de Shanno Fano\n";
	descompactarShannoFano();
	cout << "Descompactacao por Shanno Fano concluida\n";
}

int main ()
{
	cout << "Bem vindo ao compactador desenvolvidor por Felipe Oliveira - Ikhor\n";
	cout << "Neste compactador estão implementados os algoritmo RLE,Shanno Fano e Huffaman\n";
	cout << "o unico requisito é que o arquivo deverá estar com nome de wow.bmp para pode ser compactador\n";
	int entrada = 5;
	while(entrada != 0){
	cout << "Digite 1 para codificar com RLE\n";
	cout << "Digite 2 para decodificar com RLE\n";
	cout << "Digite 3 para codificar com Huffman\n";
	cout << "Digite 4 para decodificar com Huffman\n";
	cout << "Digite 5 para codificar com Shanno Fano\n";
	cout << "Digite 6 para decodificar com Shanno Fano\n";
	cout << "Digite 7 para comparar o arquivo original e o decodificado\n";
	cout << "Digite 0 para sair\n";
	cin >> entrada;
	if(entrada == 1)
		RLEC();
	if(entrada == 2)
		RLED();
	if(entrada == 3)
		HuffmanC();
	if(entrada == 4)
		HuffmanD();
	if(entrada == 5)
		ShannoFanoC();
	if(entrada == 6)
		ShannoFanoD();
	if(entrada == 7)
		comparaArquivos();
	}
	system("pause");
}
