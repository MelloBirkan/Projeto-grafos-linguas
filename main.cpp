/*
╔════════════════════════════════════════════════════════════╗
║ Feito pelos alunos:                                              ║
║ • Marcello Gonzatto Birkan (10381938)                            ║
║ • Daniela Brazolin Flauto (10395891)                             ║
╚════════════════════════════════════════════════════════════╝
*/
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <clocale>
#include <codecvt>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <numeric>
#include <queue>
#include <set>
#include <tuple>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cwchar>

using namespace std;

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

namespace {
constexpr int MAX_SPINNER_CYCLES = 12;

string trim(const string &texto) {
  size_t inicio = texto.find_first_not_of(" \t\n\r");
  if (inicio == string::npos) {
    return "";
  }
  size_t fim = texto.find_last_not_of(" \t\n\r");
  return texto.substr(inicio, fim - inicio + 1);
}

string normalizarTexto(const string &texto) {
  string resultado;
  bool espacoAnterior = true;
  for (char ch : texto) {
    unsigned char c = static_cast<unsigned char>(ch);
    if (isalnum(c)) {
      resultado.push_back(static_cast<char>(tolower(c)));
      espacoAnterior = false;
    } else if (isspace(c) || ch == '_' || ch == '-' || ch == '/') {
      if (!espacoAnterior && !resultado.empty()) {
        resultado.push_back(' ');
        espacoAnterior = true;
      }
    }
  }
  if (!resultado.empty() && resultado.back() == ' ') {
    resultado.pop_back();
  }
  return resultado;
}

bool ehNumeroInteiro(const string &texto) {
  if (texto.empty())
    return false;
  for (char ch : texto) {
    if (!isdigit(static_cast<unsigned char>(ch))) {
      return false;
    }
  }
  return true;
}

bool ehEmojiLargo(char32_t ch) {
  if (ch >= 0x1F300 && ch <= 0x1FAFF)
    return true;
  if (ch >= 0x1F1E6 && ch <= 0x1F1FF)
    return true; // bandeiras
  if (ch >= 0x2600 && ch <= 0x26FF)
    return true;
  if (ch >= 0x2700 && ch <= 0x27BF)
    return true;
  return false;
}

int larguraVisual(const string &texto) {
  static wstring_convert<codecvt_utf8<char32_t>, char32_t> converter;
  int largura = 0;
  u32string u32 = converter.from_bytes(texto);
  for (char32_t ch : u32) {
    if (ch == 0xFE0F)
      continue; // variation selector
    int w = wcwidth(static_cast<wchar_t>(ch));
    if (w < 0)
      w = 1;
    if (w == 1 && ehEmojiLargo(ch))
      w = 2;
    largura += w;
  }
  return largura;
}

string alinharMenu(const string &texto, int largura) {
  int larguraTexto = larguraVisual(texto);
  int espacos = max(0, largura - larguraTexto);
  return texto + string(espacos, ' ');
}

string repetirSimbolo(const string &valor, int vezes) {
  string resultado;
  if (vezes <= 0)
    return resultado;
  resultado.reserve(static_cast<size_t>(valor.size()) * vezes);
  for (int i = 0; i < vezes; ++i) {
    resultado += valor;
  }
  return resultado;
}
} // namespace

class Grafo {
private:
  int tipo = 0;
  int numVertices = 0;
  int numArestas = 0;
  bool carregado = false;

  vector<string> nomeVertices;
  vector<vector<pair<int, int>>> adj;
  vector<string> grupoVertices;
  vector<int> grauVertices;

  unordered_map<string, string> corGrupo{{"LUS", GREEN}, {"HIS", YELLOW},
                                         {"ANG", BLUE},  {"FRA", MAGENTA},
                                         {"ARA", CYAN},  {"OUT", RESET}};

  unordered_map<string, string> nomeGrupo{
      {"LUS", "Lusófonos"},   {"HIS", "Hispanófonos"}, {"ANG", "Anglófonos"},
      {"FRA", "Francófonos"}, {"ARA", "Arabófonos"},   {"OUT", "Outros"}};

  bool grafoCarregado() const {
    if (!carregado) {
      cout << YELLOW << "⚠️  Carregue o grafo primeiro (Opção 1)." << RESET
           << '\n';
      return false;
    }
    return true;
  }

  bool indiceValido(int id) const { return id >= 0 && id < numVertices; }

  void mostrarProgresso(int atual, int total) {
    if (total == 0)
      return;
    int porcentagem =
        static_cast<int>((static_cast<double>(atual) / total) * 100.0);
    int blocos = porcentagem / 5;
    cout << '\r' << "[";
    for (int i = 0; i < 20; ++i) {
      cout << (i < blocos ? "█" : "░");
    }
    cout << "] " << setw(3) << porcentagem << "%" << flush;
  }

  void animacaoLoading(const string &texto) {
    static const vector<string> frames = {"⠋", "⠙", "⠹", "⠸", "⠼",
                                          "⠴", "⠦", "⠧", "⠇", "⠏"};
    for (int i = 0; i < MAX_SPINNER_CYCLES; ++i) {
      cout << '\r' << frames[i % frames.size()] << ' ' << texto << flush;
      this_thread::sleep_for(chrono::milliseconds(120));
    }
    cout << '\r' << string(texto.size() + 4, ' ') << '\r';
  }

  void atualizarGruposPadrao() {
    grupoVertices.assign(numVertices, "OUT");
    for (int id = 0; id < numVertices; ++id) {
      if (id <= 8)
        grupoVertices[id] = "LUS";
      else if (id <= 28)
        grupoVertices[id] = "HIS";
      else if (id <= 39)
        grupoVertices[id] = "ANG";
      else if (id <= 48)
        grupoVertices[id] = "FRA";
      else if (id <= 55)
        grupoVertices[id] = "ARA";
      else
        grupoVertices[id] = "OUT";
    }
  }

  void recalcularGraus() {
    grauVertices.assign(numVertices, 0);
    for (int i = 0; i < numVertices; ++i) {
      grauVertices[i] = static_cast<int>(adj[i].size());
    }
  }

  vector<vector<int>> componentesConexos() {
    vector<vector<int>> componentes;
    if (!grafoCarregado())
      return componentes;

    vector<int> visitado(numVertices, 0);
    queue<int> fila;

    for (int v = 0; v < numVertices; ++v) {
      if (visitado[v])
        continue;
      vector<int> componente;
      fila.push(v);
      visitado[v] = 1;

      while (!fila.empty()) {
        int atual = fila.front();
        fila.pop();
        componente.push_back(atual);

        for (const auto &viz : adj[atual]) {
          if (!visitado[viz.first]) {
            visitado[viz.first] = 1;
            fila.push(viz.first);
          }
        }
      }

      componentes.push_back(componente);
    }

    return componentes;
  }

  pair<int, vector<int>> dijkstra(int origem, int destino) {
    const int INF = numeric_limits<int>::max();
    vector<int> dist(numVertices, INF);
    vector<int> anterior(numVertices, -1);
    typedef pair<int, int> Item; // distancia, vertice
    priority_queue<Item, vector<Item>, greater<Item>> fila;

    dist[origem] = 0;
    fila.push({0, origem});

    while (!fila.empty()) {
      Item topo = fila.top();
      fila.pop();

      int custo = topo.first;
      int v = topo.second;
      if (custo > dist[v])
        continue;
      if (v == destino)
        break;

      for (const auto &viz : adj[v]) {
        int u = viz.first;
        int peso = viz.second;
        if (dist[v] != INF && dist[v] + peso < dist[u]) {
          dist[u] = dist[v] + peso;
          anterior[u] = v;
          fila.push(Item(dist[u], u));
        }
      }
    }

    vector<int> caminho;
    if (dist[destino] == INF) {
      return {INF, caminho};
    }

    for (int v = destino; v != -1; v = anterior[v]) {
      caminho.push_back(v);
    }
    reverse(caminho.begin(), caminho.end());

    return {dist[destino], caminho};
  }

  string corParaGrupo(const string &grupo) const {
    auto it = corGrupo.find(grupo);
    return it != corGrupo.end() ? it->second : RESET;
  }

  string nomeParaGrupo(const string &grupo) const {
    auto it = nomeGrupo.find(grupo);
    return it != nomeGrupo.end() ? it->second : "Outros";
  }

public:
  void limparTela() { cout << "\033[2J\033[H"; }

  void mostrarAjuda() {
    limparTela();
    cout << CYAN
         << "╔════════════════════════════════════════════════════════════╗\n"
         << "║ ❓  CENTRAL DE AJUDA E DOCUMENTAÇÃO                        ║\n"
         << "╚════════════════════════════════════════════════════════════╝\n"
         << RESET;

    cout << "\n" << BOLD << "📌 VISÃO GERAL DO SISTEMA" << RESET << "\n";
    cout << "   Este software utiliza a Teoria dos Grafos para analisar barreiras e conexões\n"
         << "   linguísticas entre países. Cada " << BOLD << "Vértice" << RESET << " é um país e cada " << BOLD << "Aresta" << RESET << " representa\n"
         << "   o grau de proximidade linguística (peso menor = maior similaridade).\n\n";

    cout << BOLD << "📂 OPERAÇÕES BÁSICAS" << RESET << "\n";
    cout << "   " << CYAN << "[1] Ler dados:" << RESET << " Carrega a base de dados 'grafo.txt'. " << YELLOW << "(Faça isso primeiro!)" << RESET << "\n";
    cout << "   " << CYAN << "[2] Gravar dados:" << RESET << " Salva todas as alterações feitas na memória para o arquivo.\n";
    cout << "   " << CYAN << "[7] Ver arquivo:" << RESET << " Exibe o conteúdo bruto do arquivo de texto carregado.\n";
    cout << "   " << CYAN << "[0] Sair:" << RESET << " Encerra o programa.\n\n";

    cout << BOLD << "✏️  EDIÇÃO DO GRAFO" << RESET << "\n";
    cout << "   " << CYAN << "[3] Inserir Vértice:" << RESET << " Adiciona um novo país ao sistema.\n";
    cout << "   " << CYAN << "[4] Inserir Aresta:" << RESET << " Cria uma conexão entre dois países.\n";
    cout << "       " << YELLOW << "Exemplo:" << RESET << " Conectar Brasil -> Portugal com peso 5 (muito próximos).\n";
    cout << "   " << CYAN << "[5/6] Remover:" << RESET << " Exclui países ou conexões existentes.\n\n";

    cout << BOLD << "📊 ANÁLISE E VISUALIZAÇÃO" << RESET << "\n";
    cout << "   " << CYAN << "[8] Mostrar Grafo:" << RESET << " Lista visual de todos os países e seus vizinhos.\n";
    cout << "   " << CYAN << "[9] Conexidade:" << RESET << " Verifica se o grafo é conexo (todos alcançam todos) e\n";
    cout << "       mostra a matriz reduzida de conexões entre grupos (ex: Lusófonos vs Anglófonos).\n";
    cout << "   " << CYAN << "[10] Estatísticas:" << RESET << " Exibe os maiores 'Hubs' (países mais conectados) e\n";
    cout << "        análise de pontes (conexões vitais entre grupos diferentes).\n";
    cout << "   " << CYAN << "[12] Buscar:" << RESET << " Localiza o ID de um país pelo nome (busca parcial suportada).\n\n";

    cout << BOLD << "🚀 FERRAMENTAS AVANÇADAS (PROJETO 2)" << RESET << "\n";
    cout << "   " << CYAN << "[11] Compatibilidade:" << RESET << " Lista países do mesmo grupo que não precisam de tradução.\n";
    cout << "   " << CYAN << "[13] Coloração:" << RESET << " Divide os países em grupos de cores onde vizinhos nunca têm a\n";
    cout << "        mesma cor (útil para alocação de recursos ou frequências).\n";
    cout << "   " << CYAN << "[14] Rota Ótima (Dijkstra):" << RESET << " Calcula o caminho mais 'barato' entre dois países.\n";
    cout << "        Usa o peso das arestas como 'custo' de tradução/adaptação.\n";
    cout << "   " << CYAN << "[15] Alcance de Mercado:" << RESET << " Simula a expansão de um produto/mensagem.\n";
    cout << "        " << BOLD << "• Por Custo:" << RESET << " Até onde chego com um orçamento X?\n";
    cout << "        " << BOLD << "• Por Saltos:" << RESET << " Até onde chego traduzindo no máximo N vezes?\n";
  }

  void mostrarCabecalho() {
    limparTela();
    cout << CYAN
         << "╔════════════════════════════════════════════════════════════╗\n"
         << "║ 🌍  SISTEMA DE EXPANSÃO LINGUÍSTICA INTERNACIONAL  🌍     ║\n"
         << "║         Análise de Barreiras Linguísticas v1.0            ║\n"
         << "╚════════════════════════════════════════════════════════════╝\n"
         << RESET;
  }

  void mostrarMenuPrincipal() {
    cout << YELLOW << "\n📋 MENU PRINCIPAL:\n" << RESET;
    constexpr int larguraInterna = 60;
    const string linha = repetirSimbolo("─", larguraInterna);
    auto escreverLinha = [&](const string &conteudo) {
      cout << "│ " << alinharMenu(conteudo, larguraInterna - 2) << " │\n";
    };

    cout << "┌" << linha << "┐\n";
    escreverLinha("[1] 📂 Ler dados do arquivo grafo.txt");
    escreverLinha("[2] 💾 Gravar dados no arquivo grafo.txt");
    escreverLinha("[3] ➕ Inserir vértice (país)");
    escreverLinha("[4] 🔗 Inserir aresta (conexão)");
    escreverLinha("[5] ➖ Remover vértice");
    escreverLinha("[6] ✂️ Remover aresta");
    escreverLinha("[7] 📄 Mostrar conteúdo do arquivo");
    escreverLinha("[8] 📊 Mostrar grafo (lista)");
    escreverLinha("[9] 🔍 Apresentar conexidade e reduzido");
    escreverLinha("");
    escreverLinha("[10] 📈 Estatísticas e análises");
    escreverLinha("[11] 🛒 Compatibilidade do produto (sem tradução)");
    escreverLinha("[12] 🏷️ Buscar país por ID/Nome");
    escreverLinha("");
    escreverLinha("🆕 NOVOS RECURSOS (Projeto 2):");
    escreverLinha("[13] 🎨 Coloração do Grafo e Número Cromático");
    escreverLinha("[14] 🚀 Rota linguística ótima (Dijkstra)");
    escreverLinha("[15] 🌐 Alcance de mercado (custo/saltos)");
    escreverLinha("");
    escreverLinha("[99] ❓ Ajuda e Exemplos");
    escreverLinha("[0] 🚪 Encerrar aplicação");
    cout << "└" << linha << "┘\n";
  }

  void mostrarConteudoArquivo(const string &nomeArquivo) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
      cout << RED << "❌ Não foi possível abrir o arquivo." << RESET << '\n';
      return;
    }

    cout << CYAN << "\n📄 Conteúdo de " << nomeArquivo << ":\n" << RESET;
    string linha;
    while (getline(arquivo, linha)) {
      cout << linha << '\n';
    }
  }

  void lerArquivo(const string &nomeArquivo) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
      cout << RED << "❌ Arquivo não encontrado." << RESET << '\n';
      return;
    }

    cout << YELLOW << "⏳ Carregando arquivo";
    cout.flush();
    for (int i = 0; i < 3; ++i) {
      this_thread::sleep_for(chrono::milliseconds(350));
      cout << "." << flush;
    }
    cout << '\n';

    int novoTipo = 0;
    int novoNumVertices = 0;
    int novoNumArestas = 0;

    if (!(arquivo >> novoTipo)) {
      cout << RED << "❌ Arquivo inválido." << RESET << '\n';
      return;
    }
    if (!(arquivo >> novoNumVertices)) {
      cout << RED << "❌ Não foi possível ler o número de vértices." << RESET
           << '\n';
      return;
    }

    string linha;
    getline(arquivo, linha); // consumir fim de linha

    vector<string> novosNomes(novoNumVertices);
    for (int i = 0; i < novoNumVertices; ++i) {
      if (!getline(arquivo, linha)) {
        cout << RED << "❌ Falha ao ler nome de vértice." << RESET << '\n';
        return;
      }
      if (linha.empty()) {
        --i;
        continue;
      }

      stringstream ss(linha);
      int id = -1;
      ss >> id;
      ss >> ws;

      string nome;
      if (!ss.good()) {
        cout << RED << "❌ Linha de vértice inválida." << RESET << '\n';
        return;
      }

      if (ss.peek() == '"') {
        ss.get();
        getline(ss, nome, '"');
      } else {
        getline(ss, nome);
      }

      nome = trim(nome);
      novosNomes[i] = nome;
    }

    if (!(arquivo >> novoNumArestas)) {
      cout << RED << "❌ Não foi possível ler o número de arestas." << RESET
           << '\n';
      return;
    }

    vector<vector<pair<int, int>>> novaAdj(novoNumVertices);
    int v1, v2, peso;
    int lidas = 0;

    while (arquivo >> v1 >> v2 >> peso) {
      if (v1 < 0 || v1 >= novoNumVertices || v2 < 0 || v2 >= novoNumVertices) {
        cout << RED << "❌ Aresta inválida encontrada." << RESET << '\n';
        return;
      }
      novaAdj[v1].push_back({v2, peso});
      if (novoTipo == 2) {
        novaAdj[v2].push_back({v1, peso});
      }
      ++lidas;
      mostrarProgresso(lidas, novoNumArestas);
    }
    cout << '\n';

    tipo = novoTipo;
    numVertices = novoNumVertices;
    numArestas = novoNumArestas;
    nomeVertices = std::move(novosNomes);
    adj = std::move(novaAdj);
    atualizarGruposPadrao();
    recalcularGraus();
    carregado = true;

    cout << GREEN << "✅ Grafo carregado com sucesso!" << RESET << '\n';
    cout << CYAN << "📊 Estatísticas:\n" << RESET;
    double densidade = 0.0;
    if (numVertices > 1) {
      densidade = (static_cast<double>(numArestas) * 100.0) /
                  (static_cast<double>(numVertices) * (numVertices - 1) /
                   (tipo == 2 ? 2.0 : 1.0));
    }
    cout << "   • Países: " << numVertices << '\n';
    cout << "   • Conexões: " << numArestas << '\n';
    cout << fixed << setprecision(2);
    cout << "   • Densidade: " << densidade << "%\n";
    cout.unsetf(ios::fixed);
    cout << setprecision(6);
  }

  void gravarArquivo(const string &nomeArquivo) {
    if (!grafoCarregado())
      return;

    ofstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
      cout << RED << "❌ Não foi possível gravar o arquivo." << RESET << '\n';
      return;
    }

    arquivo << tipo << '\n';
    arquivo << numVertices << '\n';
    for (int i = 0; i < numVertices; ++i) {
      arquivo << i << " \"" << nomeVertices[i] << "\"\n";
    }

    arquivo << numArestas << '\n';
    set<pair<int, int>> emitidas;
    for (int i = 0; i < numVertices; ++i) {
      for (const auto &viz : adj[i]) {
        int destino = viz.first;
        int peso = viz.second;
        auto chave = minmax(i, destino);
        if (emitidas.insert(chave).second) {
          arquivo << chave.first << ' ' << chave.second << ' ' << peso << '\n';
        }
      }
    }

    cout << GREEN << "✅ Arquivo gravado com sucesso." << RESET << '\n';
  }

  void inserirVertice(const string &nome, const string &grupo) {
    if (!grafoCarregado()) {
      cout << RED << "❌ Carregue o grafo antes de inserir vértices." << RESET
           << '\n';
      return;
    }

    for (const string &existente : nomeVertices) {
      if (existente == nome) {
        cout << RED << "❌ País já cadastrado." << RESET << '\n';
        return;
      }
    }

    nomeVertices.push_back(nome);
    adj.emplace_back();
    grupoVertices.push_back(grupo);
    grauVertices.push_back(0);
    ++numVertices;

    cout << GREEN << "✅ País adicionado com ID: " << (numVertices - 1) << RESET
         << '\n';
  }

  void inserirAresta(int v1, int v2, int peso) {
    if (!grafoCarregado())
      return;
    if (!indiceValido(v1) || !indiceValido(v2)) {
      cout << RED << "❌ IDs informados são inválidos." << RESET << '\n';
      return;
    }
    if (v1 == v2) {
      cout << RED << "❌ Não é possível criar laço no mesmo país." << RESET
           << '\n';
      return;
    }
    if (peso <= 0) {
      cout << RED << "❌ O peso deve ser positivo." << RESET << '\n';
      return;
    }

    auto &lista1 = adj[v1];
    auto &lista2 = adj[v2];
    auto existe = [&](const vector<pair<int, int>> &lista, int destino) {
      return any_of(lista.begin(), lista.end(), [&](const pair<int, int> &p) {
        return p.first == destino;
      });
    };

    if (existe(lista1, v2)) {
      cout << YELLOW << "⚠️  Conexão já existente." << RESET << '\n';
      return;
    }

    lista1.push_back({v2, peso});
    lista2.push_back({v1, peso});
    ++numArestas;
    recalcularGraus();
    cout << GREEN << "✅ Conexão criada com sucesso." << RESET << '\n';
  }

  void removerAresta(int v1, int v2) {
    if (!grafoCarregado())
      return;
    if (!indiceValido(v1) || !indiceValido(v2)) {
      cout << RED << "❌ IDs informados são inválidos." << RESET << '\n';
      return;
    }
    if (v1 == v2) {
      cout << RED << "❌ Não há arestas para o mesmo vértice." << RESET << '\n';
      return;
    }

    auto removeConexao = [](vector<pair<int, int>> &lista, int destino) {
      auto it =
          remove_if(lista.begin(), lista.end(), [&](const pair<int, int> &p) {
            return p.first == destino;
          });
      bool removido = it != lista.end();
      lista.erase(it, lista.end());
      return removido;
    };

    bool removida1 = removeConexao(adj[v1], v2);
    bool removida2 = removeConexao(adj[v2], v1);

    if (removida1 && removida2) {
      --numArestas;
      recalcularGraus();
      cout << GREEN << "✅ Conexão removida." << RESET << '\n';
    } else {
      cout << YELLOW << "⚠️  Conexão inexistente." << RESET << '\n';
    }
  }

  void removerVertice(int id) {
    if (!grafoCarregado())
      return;
    if (!indiceValido(id)) {
      cout << RED << "❌ País ID " << id << " não existe." << RESET << '\n';
      return;
    }

    int arestasRemovidas = static_cast<int>(adj[id].size());

    for (int v = 0; v < numVertices; ++v) {
      if (v == id)
        continue;
      auto &lista = adj[v];
      lista.erase(
          remove_if(lista.begin(), lista.end(),
                    [&](const pair<int, int> &p) { return p.first == id; }),
          lista.end());
    }

    adj.erase(adj.begin() + id);
    nomeVertices.erase(nomeVertices.begin() + id);
    grupoVertices.erase(grupoVertices.begin() + id);

    for (auto &lista : adj) {
      for (auto &par : lista) {
        if (par.first > id) {
          --par.first;
        }
      }
    }

    numVertices--;
    numArestas -= arestasRemovidas;
    if (numArestas < 0)
      numArestas = 0;
    recalcularGraus();

    cout << GREEN << "✅ País removido." << RESET << '\n';
  }

  void mostrarGrafo() {
    if (!grafoCarregado())
      return;

    cout << CYAN << "\n🗺️  MAPA DE CONEXÕES LINGUÍSTICAS\n" << RESET;
    cout << "════════════════════════════════════\n\n";

    unordered_map<string, vector<int>> grupos;
    for (int i = 0; i < numVertices; ++i) {
      grupos[grupoVertices[i]].push_back(i);
    }

    for (const auto &entrada : grupos) {
      const string &grupo = entrada.first;
      const auto &indices = entrada.second;
      cout << corParaGrupo(grupo) << "🟢 " << nomeParaGrupo(grupo) << ":"
           << RESET << '\n';
      for (int v : indices) {
        cout << "  [" << setw(2) << v << "] " << nomeVertices[v] << " → ";
        if (adj[v].empty()) {
          cout << "(sem conexões)";
        } else {
          bool primeiro = true;
          for (const auto &viz : adj[v]) {
            if (!primeiro)
              cout << ", ";
            cout << nomeVertices[viz.first] << "(" << viz.second << ")";
            primeiro = false;
          }
        }
        cout << '\n';
      }
      cout << '\n';
    }
  }

  void apresentarConexidade() {
    if (!grafoCarregado())
      return;

    auto componentes = componentesConexos();
    bool conexo = componentes.size() <= 1;
    size_t maiorComp = 0;
    size_t menorComp = componentes.empty() ? 0 : componentes.front().size();
    int isolados = 0;
    for (const auto &comp : componentes) {
      maiorComp = max(maiorComp, comp.size());
      menorComp = min(menorComp, comp.size());
      if (comp.size() == 1)
        ++isolados;
    }

    auto formatoPercentual = [&](size_t valor) {
      if (numVertices == 0)
        return string("0.0");
      ostringstream oss;
      oss << fixed << setprecision(1)
          << (static_cast<double>(valor) * 100.0 / numVertices);
      return oss.str();
    };

    cout << MAGENTA
         << "\n╔════════════════════════════════════════════════════════════════╗\n"
         << "║ 🔬  ANÁLISE DE CONEXIDADE LINGUÍSTICA                          ║\n"
         << "╚════════════════════════════════════════════════════════════════╝\n"
         << RESET;

    cout << (conexo ? GREEN : RED)
         << (conexo ? "✅ Grafo CONEXO" : "❌ Grafo DESCONEXO") << RESET
         << '\n';
    if (conexo) {
      cout << "   Todos os países estão interligados linguisticamente.\n";
    } else {
      cout << "   Existem " << componentes.size()
           << " grupos distintos de comunicação.\n";
    }

    cout << CYAN << "\n📌 Resumo Estrutural\n" << RESET;
    cout << "   • Países analisados: " << numVertices << '\n';
    cout << "   • Conexões linguísticas: " << numArestas << '\n';
    cout << "   • Componentes: " << componentes.size()
         << " (" << isolados << " isolados)\n";
    if (!componentes.empty()) {
      cout << "   • Maior componente: " << maiorComp << " países ("
           << formatoPercentual(maiorComp) << "%)\n";
      cout << "   • Menor componente: " << menorComp << " países ("
           << formatoPercentual(menorComp) << "%)\n";
    }

    cout << CYAN << "\n🧭 Mapa de Componentes\n" << RESET;
    cout << "   ┌────┬────────────────────┬─────────────────────────────┐\n";
    cout << "   │ ID │ Tamanho            │ Distribuição                │\n";
    cout << "   ├────┼────────────────────┼─────────────────────────────┤\n";

    const size_t larguraBarra = 20;
    const string blocoCheio = u8"█";
    const string blocoVazio = u8"░";
    auto repetir = [](const string &valor, size_t vezes) {
      string resultado;
      resultado.reserve(valor.size() * vezes);
      for (size_t i = 0; i < vezes; ++i)
        resultado += valor;
      return resultado;
    };

    for (size_t indice = 0; indice < componentes.size(); ++indice) {
      const auto &comp = componentes[indice];
      size_t tamanho = comp.size();
      size_t barrasPreenchidas = max<size_t>(1, tamanho * larguraBarra /
                                                     max(1, numVertices));
      barrasPreenchidas = min(larguraBarra, barrasPreenchidas);
      string barra = repetir(blocoCheio, barrasPreenchidas) +
                     repetir(blocoVazio, larguraBarra - barrasPreenchidas);

      cout << "   │    │ " << left << setw(18)
           << (formatoPercentual(tamanho) + "% do grafo") << right
           << " │ ";
      size_t contador = 0;
      for (size_t j = 0; j < comp.size(); ++j) {
        int id = comp[j];
        string rotulo = corParaGrupo(grupoVertices[id]) + nomeVertices[id] + RESET + "[" + to_string(id) + "]";
        cout << rotulo;
        if (j + 1 < comp.size())
          cout << ", ";
        if (++contador % 3 == 0 && j + 1 < comp.size())
          cout << "\n   │    │ " << setw(18) << " " << " │ ";
      }
      if (comp.empty())
        cout << "(vazio)";
      cout << '\n';
      cout << "   ├────┼────────────────────┼─────────────────────────────┤\n";
    }
    if (!componentes.empty()) {
      cout << "   └────┴────────────────────┴─────────────────────────────┘\n";
    } else {
      cout << "   │ -- │ Sem dados           │ Carregue um grafo primeiro │\n";
      cout << "   └────┴────────────────────┴─────────────────────────────┘\n";
    }

    mostrarGrafoReduzido();
  }

  void mostrarGrafoReduzido() {
    if (!grafoCarregado())
      return;

    unordered_map<string, int> indiceGrupo;
    vector<string> ordemGrupos;
    for (const auto &g : grupoVertices) {
      if (!indiceGrupo.count(g)) {
        indiceGrupo[g] = static_cast<int>(ordemGrupos.size());
        ordemGrupos.push_back(g);
      }
    }

    sort(ordemGrupos.begin(), ordemGrupos.end(), [&](const string &a,
                                                     const string &b) {
      return nomeParaGrupo(a) < nomeParaGrupo(b);
    });
    indiceGrupo.clear();
    for (size_t idx = 0; idx < ordemGrupos.size(); ++idx) {
      indiceGrupo[ordemGrupos[idx]] = static_cast<int>(idx);
    }

    int gCount = static_cast<int>(ordemGrupos.size());
    vector<vector<int>> matriz(gCount, vector<int>(gCount, 0));
    int maxValor = 0;

    set<pair<int, int>> vistos;
    for (int i = 0; i < numVertices; ++i) {
      for (const auto &viz : adj[i]) {
        auto chave = minmax(i, viz.first);
        if (vistos.insert(chave).second) {
          int gi = indiceGrupo[grupoVertices[chave.first]];
          int gj = indiceGrupo[grupoVertices[chave.second]];
          matriz[gi][gj] += viz.second;
          if (gi != gj) {
            matriz[gj][gi] += viz.second;
          }
          maxValor = max(maxValor, matriz[gi][gj]);
          maxValor = max(maxValor, matriz[gj][gi]);
        }
      }
    }

    const vector<string> gradacao = {"···", "░░░", "▒▒▒", "▓▓▓", "███"};
    const int rotuloLargura = 20;
    const int celulaLargura = 17;
    const int barraLargura = 4;
    const string linha = u8"─";
    const string coluna = u8"│";
    const string cantoSupEsq = u8"┌";
    const string cantoSupDir = u8"┐";
    const string cantoInfEsq = u8"└";
    const string cantoInfDir = u8"┘";
    const string cruzTopo = u8"┬";
    const string cruzMeio = u8"┼";
    const string cruzBase = u8"┴";

    auto repetir = [](const string &valor, size_t vezes) {
      string resultado;
      resultado.reserve(valor.size() * vezes);
      for (size_t i = 0; i < vezes; ++i)
        resultado += valor;
      return resultado;
    };

    auto desenharLinha = [&](const string &left, const string &mid,
                             const string &right) {
      cout << "    " << left << repetir(linha, rotuloLargura + 2);
      for (int c = 0; c < gCount; ++c) {
        cout << mid << repetir(linha, celulaLargura + 2);
      }
      cout << right << '\n';
    };

    auto campo = [&](const string &texto, int largura) {
      ostringstream oss;
      oss << left << setw(largura) << texto;
      string base = oss.str();
      return string(" ") + base + " ";
    };

    auto formatarCelula = [&](int valor) {
      double proporcao = 0.0;
      int escala = 0;
      if (valor > 0 && maxValor > 0) {
        proporcao = static_cast<double>(valor) / maxValor;
        escala = static_cast<int>(ceil(proporcao * 4.0));
        escala = max(1, min(4, escala));
      }
      int preenchidos = 0;
      if (valor > 0 && maxValor > 0) {
        preenchidos = static_cast<int>(round(proporcao * barraLargura));
        preenchidos = max(1, min(barraLargura, preenchidos));
      }
      string barra = "[";
      barra += string(preenchidos, '=');
      barra += string(barraLargura - preenchidos, ' ');
      barra += "]";

      ostringstream oss;
      oss << setw(6) << valor << " " << barra << " " << gradacao[escala];
      return oss.str();
    };

    cout << CYAN << "\n🌐 MATRIZ DE CONEXÕES ENTRE GRUPOS\n" << RESET;
    if (gCount == 0) {
      cout << YELLOW << "   ⚠️  Nenhum grupo disponível para exibir." << RESET
           << '\n';
      return;
    }

    desenharLinha(cantoSupEsq, cruzTopo, cantoSupDir);
    cout << "    " << coluna << campo("Grupo origem", rotuloLargura);
    for (const auto &grupo : ordemGrupos) {
      cout << coluna << campo(nomeParaGrupo(grupo), celulaLargura);
    }
    cout << coluna << '\n';
    desenharLinha(u8"├", cruzMeio, u8"┤");

    for (int i = 0; i < gCount; ++i) {
      cout << "    " << coluna << " " << corParaGrupo(ordemGrupos[i]) << left
           << setw(rotuloLargura) << nomeParaGrupo(ordemGrupos[i]) << RESET
           << right << " ";
      for (int j = 0; j < gCount; ++j) {
        cout << coluna << campo(formatarCelula(matriz[i][j]), celulaLargura);
      }
      cout << coluna << '\n';
      if (i + 1 < gCount)
        desenharLinha(u8"├", cruzMeio, u8"┤");
    }
    desenharLinha(cantoInfEsq, cruzBase, cantoInfDir);

    cout << CYAN << "    Legenda:" << RESET << " " << BOLD << gradacao[4]
         << RESET << " forte  " << gradacao[3] << " moderada  "
         << gradacao[2] << " leve  " << gradacao[1] << " fraca  "
         << gradacao[0] << " sem ligação" << '\n';
    cout << "    " << "Valores normalizados pela maior conexão do mapa." << '\n';
  }

  void mostrarGruposLinguisticos() {
    if (!grafoCarregado())
      return;

    unordered_map<string, vector<int>> agrupados;
    for (int i = 0; i < numVertices; ++i) {
      agrupados[grupoVertices[i]].push_back(i);
    }

    cout << CYAN << "\n🌍 Grupos linguísticos:\n" << RESET;
    for (const auto &entrada : agrupados) {
      const string &grupo = entrada.first;
      const auto &lista = entrada.second;
      cout << corParaGrupo(grupo) << "• " << nomeParaGrupo(grupo) << " ("
           << lista.size() << ")" << RESET << '\n';
      int contador = 0;
      for (int id : lista) {
        cout << "   [" << id << "] " << nomeVertices[id];
        if (++contador % 3 == 0)
          cout << '\n';
        else
          cout << "   ";
      }
      if (contador % 3 != 0)
        cout << '\n';
    }
  }

  void mostrarEstatisticas() {
    if (!grafoCarregado())
      return;

    cout << CYAN << "\n📊 DASHBOARD ANALÍTICO\n" << RESET;

    vector<pair<int, int>> graus;
    for (int i = 0; i < numVertices; ++i) {
      graus.push_back(pair<int, int>(grauVertices[i], i));
    }
    sort(graus.begin(), graus.end(),
         [](const pair<int, int> &a, const pair<int, int> &b) {
           if (a.first != b.first)
             return a.first > b.first;
           return a.second < b.second;
         });

    cout << "\n🏆 TOP 5 HUBS LINGUÍSTICOS:\n";
    for (size_t i = 0; i < min<size_t>(5, graus.size()); ++i) {
      int grau = graus[i].first;
      int id = graus[i].second;
      int barras = min(20, grau * 2);
      cout << setw(12) << nomeVertices[id] << " [";
      for (int b = 0; b < barras; ++b)
        cout << "█";
      for (int b = barras; b < 20; ++b)
        cout << ' ';
      cout << "] " << grau << " conexões\n";
    }

    // cálculo de "pontes" focado em países (força para fora do grupo)
    // Mostrar países com maior número de conexões para fora do próprio grupo
    vector<pair<int, int>> fora; // (numPaisesFora, id)
    fora.reserve(numVertices);
    for (int v = 0; v < numVertices; ++v) {
      const string &g = grupoVertices[v];
      int numFora = 0;
      // contar vizinhos diretos de grupos diferentes (únicos)
      std::unordered_set<int> vistos;
      for (const auto &e : adj[v]) {
        int u = e.first;
        if (grupoVertices[u] != g && !vistos.count(u)) {
          vistos.insert(u);
          ++numFora;
        }
      }
      fora.push_back({numFora, v});
    }
    sort(fora.begin(), fora.end(), [&](const pair<int,int>& a, const pair<int,int>& b){
      if (a.first != b.first) return a.first > b.first;
      return nomeVertices[a.second] < nomeVertices[b.second];
    });
    cout << "\n🌉 PAÍSES COM MAIS CONEXÕES FORA DO SEU GRUPO:\n";
    int mostrar = 0;
    int maxQtd = fora.empty() ? 0 : max(0, fora.front().first);
    for (size_t i = 0; i < fora.size() && mostrar < 10; ++i) {
      int qtd = fora[i].first;
      if (qtd <= 0) break;
      int id = fora[i].second;
      int barras = maxQtd > 0 ? static_cast<int>(round(20.0 * qtd / maxQtd)) : 0;
      barras = min(20, max(1, barras));
      cout << setw(12) << nomeVertices[id] << " [";
      for (int b = 0; b < barras; ++b) cout << "█";
      for (int b = barras; b < 20; ++b) cout << ' ';
      cout << "] " << qtd << " países fora do seu grupo\n";
      ++mostrar;
    }

    if (!graus.empty()) {
      cout << "\n💡 INSIGHTS ESTRATÉGICOS:\n";
      cout << "• Melhor país para hub multilíngue: "
           << nomeVertices[graus.front().second] << '\n';
    }
  }

  void mostrarPaisPorId(int id) {
    if (!grafoCarregado())
      return;
    if (!indiceValido(id)) {
      cout << RED << "❌ País ID " << id << " não existe." << RESET << '\n';
      return;
    }

    const string &grupo = grupoVertices[id];
    cout << CYAN << "\n🏷️  DETALHES DO PAÍS\n" << RESET;
    cout << "━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "ID: " << id << '\n';
    cout << "Nome: " << nomeVertices[id] << '\n';
    cout << "Grupo linguístico: " << nomeParaGrupo(grupo) << '\n';
    cout << "Conexões diretas: " << grauVertices[id] << '\n';
  }

  int encontrarPaisPorNome(const string &entrada) {
    if (!grafoCarregado())
      return -1;
    string buscaNormalizada = normalizarTexto(entrada);
    if (buscaNormalizada.empty())
      return -1;

    for (int i = 0; i < numVertices; ++i) {
      if (normalizarTexto(nomeVertices[i]) == buscaNormalizada) {
        return i;
      }
    }

    for (int i = 0; i < numVertices; ++i) {
      string nomeNorm = normalizarTexto(nomeVertices[i]);
      if (!nomeNorm.empty() &&
          nomeNorm.find(buscaNormalizada) != string::npos) {
        return i;
      }
    }

    return -1;
  }

  int resolverEntradaVertice(const string &entrada) {
    if (!grafoCarregado())
      return -1;
    string texto = trim(entrada);
    if (texto.empty())
      return -1;

    if (ehNumeroInteiro(texto)) {
      try {
        int id = stoi(texto);
        return indiceValido(id) ? id : -1;
      } catch (const exception &) {
        return -1;
      }
    }

    return encontrarPaisPorNome(texto);
  }

  bool confirmarVertice(int id) {
    if (!grafoCarregado() || !indiceValido(id))
      return false;

    mostrarPaisPorId(id);
    while (true) {
      cout << CYAN << "Confirmar este país? (s/N): " << RESET;
      string resposta;
      getline(cin, resposta);
      resposta = normalizarTexto(resposta);

      if (resposta.empty() || resposta == "n" || resposta == "nao" ||
          resposta == "no") {
        return false;
      }
      if (resposta == "s" || resposta == "sim") {
        return true;
      }
      if (resposta == "cancelar" || resposta == "sair") {
        return false;
      }

      cout << YELLOW
           << "⚠️  Resposta inválida. Digite 's' para sim ou 'n' para não."
           << RESET << '\n';
    }
  }

  void buscarCaminhoLinguistico(int origem, int destino) {
    if (!grafoCarregado())
      return;
    if (!indiceValido(origem) || !indiceValido(destino)) {
      cout << RED << "❌ IDs inválidos." << RESET << '\n';
      return;
    }

    animacaoLoading("Calculando rota otimizada...");
    auto resultado = dijkstra(origem, destino);
    if (resultado.first == numeric_limits<int>::max()) {
      cout << RED << "❌ Não há rota entre os países informados." << RESET
           << '\n';
      return;
    }

    cout << GREEN << "✅ ROTA ENCONTRADA!" << RESET << '\n';
    cout << "━━━━━━━━━━━━━━━━━━━━━\n";
    const auto &caminho = resultado.second;
    for (size_t i = 0; i < caminho.size(); ++i) {
      int id = caminho[i];
      cout << nomeVertices[id] << '\n';
      if (i + 1 < caminho.size()) {
        int proximo = caminho[i + 1];
        int peso = 0;
        for (const auto &viz : adj[id]) {
          if (viz.first == proximo) {
            peso = viz.second;
            break;
          }
        }
        cout << "  ↓ (peso: " << peso << ")\n";
      }
    }

    cout << "\n📈 Métricas da Rota:\n";
    cout << "• Distância linguística total: " << resultado.first << '\n';
    cout << "• Número de traduções necessárias: " << (caminho.size() - 1)
         << '\n';
    cout << "• Países no caminho: " << caminho.size() << '\n';
  }

  // ===== Planejador de expansão – utilitários =====
  pair<vector<int>, vector<int>> dijkstraCompleto(int origem) {
    const int INF = numeric_limits<int>::max();
    vector<int> dist(numVertices, INF), ant(numVertices, -1);
    using Item = pair<int, int>; // dist, v
    priority_queue<Item, vector<Item>, greater<Item>> pq;
    dist[origem] = 0;
    pq.push({0, origem});
    while (!pq.empty()) {
      auto topo = pq.top();
      pq.pop();
      int d = topo.first;
      int v = topo.second;
      if (d != dist[v])
        continue;
      for (const auto &e : adj[v]) {
        int u = e.first, w = e.second;
        if (dist[v] != INF && dist[v] + w < dist[u]) {
          dist[u] = dist[v] + w;
          ant[u] = v;
          pq.push({dist[u], u});
        }
      }
    }
    return {dist, ant};
  }

  vector<int> bfsSaltos(int origem) {
    vector<int> hop(numVertices, -1);
    queue<int> q;
    hop[origem] = 0;
    q.push(origem);
    while (!q.empty()) {
      int v = q.front();
      q.pop();
      for (const auto &e : adj[v]) {
        int u = e.first;
        if (hop[u] != -1)
          continue;
        hop[u] = hop[v] + 1;
        q.push(u);
      }
    }
    return hop;
  }

  void mostrarAlcancePorCusto(int origem, int maxCusto) {
    if (!grafoCarregado())
      return;
    if (!indiceValido(origem) || maxCusto < 0) {
      cout << RED << "❌ Parâmetros inválidos." << RESET << '\n';
      return;
    }
    cout << CYAN << "\n🎯 Alcance por custo (origem: " << nomeVertices[origem]
         << ", orçamento: " << maxCusto << ")\n" << RESET;
    auto pack = dijkstraCompleto(origem);
    const auto &dist = pack.first;
    vector<pair<int, int>> lista; // (dist, v)
    for (int v = 0; v < numVertices; ++v) {
      if (v == origem)
        continue;
      if (dist[v] != numeric_limits<int>::max() && dist[v] <= maxCusto)
        lista.push_back({dist[v], v});
    }
    sort(lista.begin(), lista.end(), [&](const pair<int, int> &a, const pair<int, int> &b) {
      if (a.first != b.first) return a.first < b.first;
      return nomeVertices[a.second] < nomeVertices[b.second];
    });
    if (lista.empty()) {
      cout << YELLOW << "   ⚠️  Nenhum país alcançado dentro do orçamento." << RESET << '\n';
      return;
    }
    for (const auto &it : lista) {
      int v = it.second; int d = it.first;
      cout << "   " << corParaGrupo(grupoVertices[v])
           << "[" << v << "] " << nomeVertices[v] << RESET
           << " (custo: " << d << ")\n";
    }
    cout << "   Total: " << lista.size() << " / " << (numVertices - 1) << " países\n";
  }

  void mostrarAlcancePorSaltos(int origem, int maxSaltos) {
    if (!grafoCarregado())
      return;
    if (!indiceValido(origem) || maxSaltos < 0) {
      cout << RED << "❌ Parâmetros inválidos." << RESET << '\n';
      return;
    }
    cout << CYAN << "\n🎯 Alcance por saltos (origem: " << nomeVertices[origem]
         << ", até " << maxSaltos << " salto(s))\n" << RESET;
    auto hops = bfsSaltos(origem);
    vector<pair<int, int>> lista; // (hops, v)
    for (int v = 0; v < numVertices; ++v) {
      if (v == origem)
        continue;
      if (hops[v] != -1 && hops[v] <= maxSaltos)
        lista.push_back({hops[v], v});
    }
    sort(lista.begin(), lista.end(), [&](const pair<int, int> &a, const pair<int, int> &b) {
      if (a.first != b.first) return a.first < b.first;
      return nomeVertices[a.second] < nomeVertices[b.second];
    });
    if (lista.empty()) {
      cout << YELLOW << "   ⚠️  Nenhum país alcançado dentro do limite de saltos." << RESET << '\n';
      return;
    }
    for (const auto &it : lista) {
      int v = it.second; int h = it.first;
      cout << "   " << corParaGrupo(grupoVertices[v])
           << "[" << v << "] " << nomeVertices[v] << RESET
           << " (saltos: " << h << ")\n";
    }
    cout << "   Total: " << lista.size() << " / " << (numVertices - 1) << " países\n";
  }

  void mostrarCompatibilidadeSemTraducao(int origem) {
    if (!grafoCarregado())
      return;
    if (!indiceValido(origem)) {
      cout << RED << "❌ Parâmetros inválidos." << RESET << '\n';
      return;
    }
    const string grupo = grupoVertices[origem];
    cout << CYAN << "\n🛡️ Compatibilidade sem tradução (origem: " << nomeVertices[origem]
         << ")\n" << RESET;
    vector<int> lista;
    for (int v = 0; v < numVertices; ++v) {
      if (v == origem) continue;
      if (grupoVertices[v] == grupo) lista.push_back(v);
    }
    sort(lista.begin(), lista.end(), [&](int a, int b){ return nomeVertices[a] < nomeVertices[b]; });
    if (lista.empty()) {
      cout << YELLOW << "   ⚠️  Nenhum país do mesmo grupo encontrado." << RESET << '\n';
      return;
    }
    for (int v : lista) {
      cout << "   " << corParaGrupo(grupoVertices[v])
           << "[" << v << "] " << nomeVertices[v] << RESET << '\n';
    }
    cout << "   Total: " << lista.size() << " país(es) compatível(is) sem tradução\n";
  }

  void colorirGrafo() {
    if (!grafoCarregado())
      return;

    cout << MAGENTA << "\n╔════════════════════════════════════════════════════════════════╗\n"
         << "║ 🎨  COLORAÇÃO DO GRAFO E NÚMERO CROMÁTICO                      ║\n"
         << "╚════════════════════════════════════════════════════════════════╝\n"
         << RESET;

    animacaoLoading("Calculando coloração do grafo...");

    // Algoritmo Greedy Coloring
    vector<int> cor(numVertices, -1); // -1 significa sem cor
    cor[0] = 0; // Primeira cor para o primeiro vértice

    // Array temporário para marcar cores disponíveis
    vector<bool> coresDisponiveis(numVertices, true);

    // Atribuir cores aos vértices restantes
    for (int v = 1; v < numVertices; ++v) {
      // Marcar cores dos vizinhos como indisponíveis
      for (const auto &viz : adj[v]) {
        int u = viz.first;
        if (cor[u] != -1) {
          coresDisponiveis[cor[u]] = false;
        }
      }

      // Encontrar a primeira cor disponível
      int corEscolhida = 0;
      for (int c = 0; c < numVertices; ++c) {
        if (coresDisponiveis[c]) {
          corEscolhida = c;
          break;
        }
      }

      cor[v] = corEscolhida;

      // Resetar cores disponíveis para o próximo vértice
      fill(coresDisponiveis.begin(), coresDisponiveis.end(), true);
    }

    // Calcular número cromático (número de cores usadas)
    int numeroCromatico = 0;
    for (int c : cor) {
      numeroCromatico = max(numeroCromatico, c + 1);
    }

    cout << GREEN << "✅ COLORAÇÃO CONCLUÍDA!\n" << RESET;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    cout << CYAN << "📊 NÚMERO CROMÁTICO: " << BOLD << numeroCromatico << RESET << " cores\n\n";
    cout << "💡 Interpretação: O grafo pode ser dividido em " << numeroCromatico
         << " partições onde nenhum país\n   dentro da mesma partição tem conexão "
         << "linguística direta.\n\n";

    // Agrupar países por cor
    unordered_map<int, vector<int>> gruposPorCor;
    for (int v = 0; v < numVertices; ++v) {
      gruposPorCor[cor[v]].push_back(v);
    }

    cout << CYAN << "🎨 DISTRIBUIÇÃO POR COR:\n" << RESET;
    cout << "   ┌────────┬─────────────────────────────────────────────────┐\n";
    cout << "   │  Cor   │ Países                                          │\n";
    cout << "   ├────────┼─────────────────────────────────────────────────┤\n";

    const vector<string> nomesCores = {
        "🔴 Vermelho", "🔵 Azul", "🟢 Verde", "🟡 Amarelo",
        "🟣 Roxo", "🟠 Laranja", "🟤 Marrom", "⚫ Preto",
        "⚪ Branco", "🔷 Azul Claro"};

    for (int c = 0; c < numeroCromatico; ++c) {
      string nomeCor =
          c < static_cast<int>(nomesCores.size()) ? nomesCores[c] : ("Cor " + to_string(c));
      const auto &paises = gruposPorCor[c];

      cout << "   │ " << left << setw(6) << nomeCor << right << " │ ";

      for (size_t i = 0; i < paises.size(); ++i) {
        int id = paises[i];
        cout << nomeVertices[id] << "[" << id << "]";
        if (i + 1 < paises.size()) {
          cout << ", ";
        }
        // Quebra de linha se muito longo
        if (i > 0 && (i + 1) % 3 == 0 && i + 1 < paises.size()) {
          cout << "\n   │        │ ";
        }
      }
      cout << '\n';
      if (c + 1 < numeroCromatico) {
        cout << "   ├────────┼─────────────────────────────────────────────────┤\n";
      }
    }
    cout << "   └────────┴─────────────────────────────────────────────────┘\n";

    // Estatísticas adicionais
    cout << CYAN << "\n📈 ESTATÍSTICAS DA COLORAÇÃO:\n" << RESET;
    cout << "   • Total de países: " << numVertices << '\n';
    cout << "   • Número cromático: " << numeroCromatico << '\n';
    cout << "   • Maior grupo de mesma cor: ";
    int maiorGrupo = 0;
    for (const auto &par : gruposPorCor) {
      maiorGrupo = max(maiorGrupo, static_cast<int>(par.second.size()));
    }
    cout << maiorGrupo << " países\n";
    cout << "   • Menor grupo de mesma cor: ";
    int menorGrupo = numVertices;
    for (const auto &par : gruposPorCor) {
      menorGrupo = min(menorGrupo, static_cast<int>(par.second.size()));
    }
    cout << menorGrupo << " país(es)\n";
  }

  void analisarPropriedadesEulerianas() {
    if (!grafoCarregado())
      return;

    cout << MAGENTA << "\n╔════════════════════════════════════════════════════════════════╗\n"
         << "║ 🔄  ANÁLISE DE PROPRIEDADES EULERIANAS                         ║\n"
         << "╚════════════════════════════════════════════════════════════════╝\n"
         << RESET;

    animacaoLoading("Analisando propriedades eulerianas...");

    // Verificar conectividade
    auto componentes = componentesConexos();
    bool conexo = componentes.size() <= 1;

    if (!conexo) {
      cout << RED << "❌ O grafo NÃO é conexo!" << RESET << '\n';
      cout << YELLOW << "   ⚠️  Um grafo desconexo não pode ter caminho ou ciclo euleriano.\n"
           << RESET;
      cout << "   • Componentes conectados: " << componentes.size() << '\n';
      return;
    }

    cout << GREEN << "✅ Grafo conexo - análise euleriana possível!\n" << RESET;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    // Contar vértices com grau ímpar
    int verticesGrauImpar = 0;
    vector<int> idsGrauImpar;

    for (int v = 0; v < numVertices; ++v) {
      if (grauVertices[v] % 2 != 0) {
        verticesGrauImpar++;
        idsGrauImpar.push_back(v);
      }
    }

    cout << CYAN << "📊 ANÁLISE DE GRAUS:\n" << RESET;
    cout << "   • Total de vértices: " << numVertices << '\n';
    cout << "   • Vértices com grau par: " << (numVertices - verticesGrauImpar)
         << '\n';
    cout << "   • Vértices com grau ímpar: " << verticesGrauImpar << '\n';

    if (verticesGrauImpar > 0 && verticesGrauImpar <= 10) {
      cout << "\n   Países com grau ímpar:\n";
      for (int id : idsGrauImpar) {
        cout << "      [" << id << "] " << nomeVertices[id] << " (grau: "
             << grauVertices[id] << ")\n";
      }
    }

    // Determinar propriedades eulerianas
    bool temCicloEuleriano = (verticesGrauImpar == 0);
    bool temCaminhoEuleriano = (verticesGrauImpar == 2);

    cout << CYAN << "\n🔍 RESULTADO DA ANÁLISE:\n" << RESET;
    cout << "   ┌─────────────────────────────┬──────────────────┐\n";
    cout << "   │ Propriedade                 │ Status           │\n";
    cout << "   ├─────────────────────────────┼──────────────────┤\n";

    cout << "   │ Ciclo Euleriano             │ ";
    if (temCicloEuleriano) {
      cout << GREEN << "✅ SIM         " << RESET;
    } else {
      cout << RED << "❌ NÃO         " << RESET;
    }
    cout << " │\n";

    cout << "   │ Caminho Euleriano           │ ";
    if (temCaminhoEuleriano) {
      cout << GREEN << "✅ SIM         " << RESET;
    } else if (temCicloEuleriano) {
      cout << GREEN << "✅ SIM*        " << RESET; // Ciclo implica caminho
    } else {
      cout << RED << "❌ NÃO         " << RESET;
    }
    cout << " │\n";

    cout << "   └─────────────────────────────┴──────────────────┘\n";

    // Explicação dos resultados
    cout << CYAN << "\n💡 INTERPRETAÇÃO:\n" << RESET;

    if (temCicloEuleriano) {
      cout << "   🎉 " << BOLD << "CICLO EULERIANO EXISTE!" << RESET << '\n';
      cout << "   → Todos os vértices têm grau par.\n";
      cout << "   → É possível percorrer TODAS as arestas exatamente uma vez\n";
      cout << "     e retornar ao ponto de partida.\n";
      cout << "   → Aplicação: Rota de tradução que visita todas as conexões\n";
      cout << "     linguísticas uma única vez e retorna ao país de origem.\n";
    } else if (temCaminhoEuleriano) {
      cout << "   ✓ " << BOLD << "CAMINHO EULERIANO EXISTE!" << RESET << '\n';
      cout << "   → Exatamente 2 vértices têm grau ímpar.\n";
      cout << "   → É possível percorrer TODAS as arestas exatamente uma vez,\n";
      cout << "     começando em um vértice de grau ímpar e terminando no outro.\n";
      cout << "   → Países de início/fim: " << nomeVertices[idsGrauImpar[0]]
           << " ↔ " << nomeVertices[idsGrauImpar[1]] << '\n';
      cout << "   → Aplicação: Rota de tradução que visita todas as conexões\n";
      cout << "     linguísticas uma única vez (sem retornar ao início).\n";
    } else {
      cout << "   ✗ " << BOLD << "NEM CAMINHO NEM CICLO EULERIANO!" << RESET
           << '\n';
      cout << "   → Mais de 2 vértices têm grau ímpar (" << verticesGrauImpar
           << " vértices).\n";
      cout << "   → NÃO é possível percorrer todas as arestas exatamente uma\n";
      cout << "     vez sem repetir algumas conexões.\n";
      cout << "   → Aplicação: Qualquer rota completa de tradução precisará\n";
      cout << "     revisitar algumas conexões linguísticas.\n";
    }

    cout << CYAN << "\n📚 TEORIA:\n" << RESET;
    cout << "   • Teorema de Euler: Um grafo conexo tem ciclo euleriano ↔\n";
    cout << "     todos os vértices têm grau par.\n";
    cout << "   • Um grafo conexo tem caminho euleriano ↔ tem exatamente\n";
    cout << "     0 ou 2 vértices de grau ímpar.\n";
  }

  void arvoreGeradoraMinimaPrim() {
    if (!grafoCarregado())
      return;

    cout << MAGENTA << "\n╔════════════════════════════════════════════════════════════════╗\n"
         << "║ 🌳  ÁRVORE GERADORA MÍNIMA (Algoritmo de Prim)                 ║\n"
         << "╚════════════════════════════════════════════════════════════════╝\n"
         << RESET;

    // Verificar conectividade
    auto componentes = componentesConexos();
    bool conexo = componentes.size() <= 1;

    if (!conexo) {
      cout << RED << "❌ O grafo NÃO é conexo!" << RESET << '\n';
      cout << YELLOW
           << "   ⚠️  Não é possível gerar uma árvore geradora mínima para\n"
           << "      um grafo desconexo. Cada componente teria sua própria AGM.\n"
           << RESET;
      cout << "   • Componentes conectados: " << componentes.size() << '\n';
      return;
    }

    animacaoLoading("Calculando árvore geradora mínima...");

    // Algoritmo de Prim
    const int INF = numeric_limits<int>::max();
    vector<bool> naMST(numVertices, false);
    vector<int> chave(numVertices, INF);
    vector<int> pai(numVertices, -1);

    // Usar priority queue: (peso, vértice)
    using Item = pair<int, int>;
    priority_queue<Item, vector<Item>, greater<Item>> pq;

    // Começar do vértice 0
    int inicio = 0;
    chave[inicio] = 0;
    pq.push({0, inicio});

    vector<tuple<int, int, int>> arestasMST; // (origem, destino, peso)
    int pesoTotal = 0;

    while (!pq.empty()) {
      int u = pq.top().second;
      pq.pop();

      if (naMST[u])
        continue;

      naMST[u] = true;

      // Adicionar aresta à MST (exceto o primeiro vértice)
      if (pai[u] != -1) {
        arestasMST.push_back({pai[u], u, chave[u]});
        pesoTotal += chave[u];
      }

      // Explorar vizinhos
      for (const auto &viz : adj[u]) {
        int v = viz.first;
        int peso = viz.second;

        if (!naMST[v] && peso < chave[v]) {
          chave[v] = peso;
          pai[v] = u;
          pq.push({chave[v], v});
        }
      }
    }

    // Calcular peso total do grafo original
    int pesoTotalGrafo = 0;
    set<pair<int, int>> arestasContadas;
    for (int u = 0; u < numVertices; ++u) {
      for (const auto &viz : adj[u]) {
        int v = viz.first;
        int peso = viz.second;
        auto aresta = minmax(u, v);
        if (arestasContadas.insert(aresta).second) {
          pesoTotalGrafo += peso;
        }
      }
    }

    cout << GREEN << "✅ ÁRVORE GERADORA MÍNIMA CALCULADA!\n" << RESET;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    cout << CYAN << "📊 ESTATÍSTICAS:\n" << RESET;
    cout << "   • Peso total da AGM: " << BOLD << pesoTotal << RESET << '\n';
    cout << "   • Peso total do grafo: " << pesoTotalGrafo << '\n';
    cout << "   • Economia: " << (pesoTotalGrafo - pesoTotal) << " ("
         << fixed << setprecision(1)
         << (100.0 * (pesoTotalGrafo - pesoTotal) / pesoTotalGrafo) << "%)\n";
    cout.unsetf(ios::fixed);
    cout << setprecision(6);
    cout << "   • Arestas na AGM: " << arestasMST.size() << '\n';
    cout << "   • Arestas removidas: " << (numArestas - arestasMST.size())
         << '\n';

    cout << CYAN << "\n🌳 ARESTAS DA ÁRVORE GERADORA MÍNIMA:\n" << RESET;
    cout << "   ┌──────────────────────────┬──────────────────────────┬────────┐\n";
    cout << "   │ País A                   │ País B                   │  Peso  │\n";
    cout << "   ├──────────────────────────┼──────────────────────────┼────────┤\n";

    // Ordenar por peso para melhor visualização (C++11-friendly)
    sort(arestasMST.begin(), arestasMST.end(),
         [](const tuple<int, int, int> &a, const tuple<int, int, int> &b) {
           return get<2>(a) < get<2>(b);
         });

    for (const auto &aresta : arestasMST) {
      int u = get<0>(aresta);
      int v = get<1>(aresta);
      int peso = get<2>(aresta);

      cout << "   │ " << left << setw(24) << nomeVertices[u] << right << " │ "
           << left << setw(24) << nomeVertices[v] << right << " │ " << setw(6)
           << peso << " │\n";
    }
    cout << "   └──────────────────────────┴──────────────────────────┴────────┘\n";

    cout << CYAN << "\n💡 INTERPRETAÇÃO:\n" << RESET;
    cout << "   A Árvore Geradora Mínima representa a rede de conexões\n";
    cout << "   linguísticas mais eficiente para conectar todos os países\n";
    cout << "   com o MENOR CUSTO TOTAL possível.\n\n";

    cout << "   🎯 Aplicação prática:\n";
    cout << "   • Expansão linguística otimizada: conecte todos os países\n";
    cout << "     usando apenas " << arestasMST.size()
         << " conexões estratégicas.\n";
    cout << "   • Economia de " << (pesoTotalGrafo - pesoTotal)
         << " unidades em custos de tradução.\n";
    cout << "   • Redução de "
         << fixed << setprecision(1)
         << (100.0 * (numArestas - arestasMST.size()) / numArestas)
         << "% nas conexões necessárias.\n";
    cout.unsetf(ios::fixed);
    cout << setprecision(6);
  }
};

static void solicitarEnter() {
  cout << '\n' << CYAN << "Pressione ENTER para continuar..." << RESET;
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int obterOpcaoMenu() {
  cout << '\n' << "Selecione uma opção: ";
  string entrada;
  getline(cin, entrada);
  if (entrada.empty())
    return -1;
  for (char c : entrada) {
    if (!isdigit(static_cast<unsigned char>(c)))
      return -1;
  }
  return stoi(entrada);
}

string escolherGrupo() {
  cout << "\nSelecione o grupo linguístico:\n";
  cout << " 1 - LUS (Lusófonos)\n";
  cout << " 2 - HIS (Hispanófonos)\n";
  cout << " 3 - ANG (Anglófonos)\n";
  cout << " 4 - FRA (Francófonos)\n";
  cout << " 5 - ARA (Arabófonos)\n";
  cout << " 6 - OUT (Outros)\n";
  cout << "Opção: ";
  string entrada;
  getline(cin, entrada);
  if (entrada.empty())
    return "OUT";
  int opcao = entrada[0] - '0';
  switch (opcao) {
  case 1:
    return "LUS";
  case 2:
    return "HIS";
  case 3:
    return "ANG";
  case 4:
    return "FRA";
  case 5:
    return "ARA";
  default:
    return "OUT";
  }
}

int solicitarVertice(Grafo &grafo, const string &prompt) {
  while (true) {
    cout << prompt;
    string entrada;
    getline(cin, entrada);
    entrada = trim(entrada);

    if (entrada.empty()) {
      cout << RED << "❌ Entrada vazia." << RESET << '\n';
      continue;
    }

    string comando = normalizarTexto(entrada);
    if (comando == "cancelar" || comando == "sair") {
      cout << YELLOW << "⚠️  Operação cancelada." << RESET << '\n';
      return -1;
    }

    int id = grafo.resolverEntradaVertice(entrada);
    if (id < 0) {
      cout << RED << "❌ País não encontrado." << RESET << '\n';
      continue;
    }

    if (grafo.confirmarVertice(id)) {
      return id;
    }

    cout << YELLOW
         << "🔁 Seleção não confirmada. Tente novamente ou digite 'cancelar'."
         << RESET << '\n';
  }
}

int main() {
  setlocale(LC_ALL, "");
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  Grafo grafo;
  bool executando = true;

  while (executando) {
    grafo.mostrarCabecalho();
    grafo.mostrarMenuPrincipal();

    int opcao = obterOpcaoMenu();
    switch (opcao) {
    case 1: {
      grafo.lerArquivo("grafo.txt");
      solicitarEnter();
      break;
    }
    case 2: {
      grafo.gravarArquivo("grafo.txt");
      solicitarEnter();
      break;
    }
    case 3: {
      cout << CYAN << "\n➕ ADICIONAR NOVO PAÍS\n" << RESET;
      cout << "Nome do país: ";
      string nome;
      getline(cin, nome);
      nome = trim(nome);
      if (nome.empty()) {
        cout << RED << "❌ Nome inválido." << RESET << '\n';
      } else {
        string grupo = escolherGrupo();
        grafo.inserirVertice(nome, grupo);
      }
      solicitarEnter();
      break;
    }
    case 4: {
      cout << CYAN << "\n🔗 INSERIR ARESTA\n" << RESET;
      int v1 = solicitarVertice(
          grafo, "Informe o país de origem (ID ou nome, ou 'cancelar'): ");
      if (v1 < 0) {
        solicitarEnter();
        break;
      }

      int v2 = solicitarVertice(
          grafo, "Informe o país de destino (ID ou nome, ou 'cancelar'): ");
      if (v2 < 0) {
        solicitarEnter();
        break;
      }

      int peso = 0;
      while (true) {
        cout << "Peso da conexão: ";
        string entradaPeso;
        getline(cin, entradaPeso);
        entradaPeso = trim(entradaPeso);
        if (entradaPeso.empty()) {
          cout << RED << "❌ Peso inválido." << RESET << '\n';
          continue;
        }
        if (!ehNumeroInteiro(entradaPeso)) {
          cout << RED << "❌ Peso deve ser numérico." << RESET << '\n';
          continue;
        }
        try {
          peso = stoi(entradaPeso);
        } catch (const exception &) {
          cout << RED << "❌ Peso inválido." << RESET << '\n';
          continue;
        }
        if (peso <= 0) {
          cout << RED << "❌ O peso deve ser positivo." << RESET << '\n';
          continue;
        }
        break;
      }

      grafo.inserirAresta(v1, v2, peso);
      solicitarEnter();
      break;
    }
    case 5: {
      cout << CYAN << "\n➖ REMOVER VÉRTICE\n" << RESET;
      int id = solicitarVertice(grafo,
                                "Informe o país (ID ou nome, ou 'cancelar'): ");
      if (id >= 0) {
        grafo.removerVertice(id);
      }
      solicitarEnter();
      break;
    }
    case 6: {
      cout << CYAN << "\n✂️  REMOVER ARESTA\n" << RESET;
      int v1 = solicitarVertice(
          grafo, "Informe o país de origem (ID ou nome, ou 'cancelar'): ");
      if (v1 < 0) {
        solicitarEnter();
        break;
      }
      int v2 = solicitarVertice(
          grafo, "Informe o país de destino (ID ou nome, ou 'cancelar'): ");
      if (v2 < 0) {
        solicitarEnter();
        break;
      }
      grafo.removerAresta(v1, v2);
      solicitarEnter();
      break;
    }
    case 7: {
      grafo.mostrarConteudoArquivo("grafo.txt");
      solicitarEnter();
      break;
    }
    case 8: {
      grafo.mostrarGrafo();
      solicitarEnter();
      break;
    }
    case 9: {
      grafo.apresentarConexidade();
      solicitarEnter();
      break;
    }
    case 10: {
      grafo.mostrarEstatisticas();
      solicitarEnter();
      break;
    }
    case 11: {
      cout << CYAN << "\n🛒 COMPATIBILIDADE DO PRODUTO (SEM TRADUÇÃO)\n" << RESET;
      int origem = solicitarVertice(
          grafo, "País de origem (ID ou nome, ou 'cancelar'): ");
      if (origem >= 0) {
        grafo.mostrarCompatibilidadeSemTraducao(origem);
      }
      solicitarEnter();
      break;
    }
    case 12: {
      cout << CYAN << "\n🏷️  CONSULTAR PAÍS\n" << RESET;
      cout << "Informe o ID ou nome do país (ou 'cancelar'): ";
      string entrada;
      getline(cin, entrada);
      entrada = trim(entrada);
      if (entrada.empty()) {
        cout << RED << "❌ Entrada vazia." << RESET << '\n';
        solicitarEnter();
        break;
      }
      string comando = normalizarTexto(entrada);
      if (comando == "cancelar" || comando == "sair") {
        cout << YELLOW << "⚠️  Consulta cancelada." << RESET << '\n';
        solicitarEnter();
        break;
      }

      int id = grafo.resolverEntradaVertice(entrada);
      if (id >= 0) {
        cout << GREEN << "🔍 País identificado (ID: " << id << ")" << RESET
             << '\n';
        grafo.mostrarPaisPorId(id);
      } else {
        cout << RED << "❌ País não encontrado." << RESET << '\n';
      }
      solicitarEnter();
      break;
    }
    case 13: {
      grafo.colorirGrafo();
      solicitarEnter();
      break;
    }
    case 14: {
      cout << CYAN << "\n🚀 ROTA LINGUÍSTICA ÓTIMA (DIJKSTRA)\n" << RESET;
      int origem = solicitarVertice(
          grafo, "País de origem (ID ou nome, ou 'cancelar'): ");
      if (origem < 0) {
        solicitarEnter();
        break;
      }
      int destino = solicitarVertice(
          grafo, "País de destino (ID ou nome, ou 'cancelar'): ");
      if (destino < 0) {
        solicitarEnter();
        break;
      }
      if (origem == destino) {
        cout << YELLOW << "⚠️  Origem e destino são o mesmo país." << RESET
             << '\n';
        solicitarEnter();
        break;
      }
      grafo.buscarCaminhoLinguistico(origem, destino);
      solicitarEnter();
      break;
    }
    case 15: {
      cout << CYAN << "\n🌐 MAPA DE ALCANCE DE MERCADO\n" << RESET;
      int origem = solicitarVertice(
          grafo, "País de partida (ID ou nome, ou 'cancelar'): ");
      if (origem < 0) {
        solicitarEnter();
        break;
      }

      cout << "\n👉 O que digitar (guia rápido):\n";
      cout << "   • Pesos das arestas vão de 0 a 100 (90-100 = mesma língua, 50-70 = próximas, 20-40 = fracas).\n";
      cout << "   • Por custo: digite um orçamento aproximado. Ex: 30 = mercados muito próximos; 60 = moderado;\n";
      cout << "     90+ = quase todos os mercados conectados pela língua chegam.\n";
      cout << "   • Por saltos: é o número de traduções em sequência. Ex: 1 = mesma língua/grupo;\n";
      cout << "     2 = usar um país-ponte; 3 = chegar em mercados mais distantes.\n\n";

      cout << "\nEscolha o critério de alcance:\n";
      cout << " 1 - Por custo (orçamento máximo em peso das arestas)\n";
      cout << " 2 - Por saltos (número máximo de traduções)\n";
      cout << "Opção [1]: ";
      string entrada;
      getline(cin, entrada);
      entrada = trim(entrada);
      if (entrada.empty())
        entrada = "1";

      int escolha = -1;
      if (ehNumeroInteiro(entrada)) {
        try {
          escolha = stoi(entrada);
        } catch (const exception &) {
          escolha = -1;
        }
      }

      bool cancelado = false;

      if (escolha == 1) {
        int orcamento = -1;
        while (orcamento < 0 && !cancelado) {
          cout << "Orçamento máximo em custo agregado (ex: 50, 120, 200, ou 'cancelar'): ";
          string ent;
          getline(cin, ent);
          ent = trim(ent);
          string cmd = normalizarTexto(ent);
          if (cmd == "cancelar" || cmd == "sair") {
            cout << YELLOW << "⚠️  Operação cancelada." << RESET << '\n';
            cancelado = true;
            break;
          }
          if (!ehNumeroInteiro(ent)) {
            cout << RED << "❌ Valor inválido." << RESET << '\n';
            continue;
          }
          try {
            orcamento = stoi(ent);
          } catch (const exception &) {
            cout << RED << "❌ Valor inválido." << RESET << '\n';
            continue;
          }
          if (orcamento < 0) {
            cout << RED << "❌ Informe um número não negativo." << RESET
                 << '\n';
          }
        }
        if (!cancelado) {
          grafo.mostrarAlcancePorCusto(origem, orcamento);
        }
      } else if (escolha == 2) {
        int saltos = -1;
        while (saltos < 0 && !cancelado) {
          cout << "Máximo de saltos/traduções consecutivas (ex: 1, 2, 3, ou 'cancelar'): ";
          string ent;
          getline(cin, ent);
          ent = trim(ent);
          string cmd = normalizarTexto(ent);
          if (cmd == "cancelar" || cmd == "sair") {
            cout << YELLOW << "⚠️  Operação cancelada." << RESET << '\n';
            cancelado = true;
            break;
          }
          if (!ehNumeroInteiro(ent)) {
            cout << RED << "❌ Valor inválido." << RESET << '\n';
            continue;
          }
          try {
            saltos = stoi(ent);
          } catch (const exception &) {
            cout << RED << "❌ Valor inválido." << RESET << '\n';
            continue;
          }
          if (saltos < 0) {
            cout << RED << "❌ Informe um número não negativo." << RESET
                 << '\n';
          }
        }
        if (!cancelado) {
          grafo.mostrarAlcancePorSaltos(origem, saltos);
        }
      } else {
        cout << RED << "❌ Opção inválida." << RESET << '\n';
      }

      solicitarEnter();
      break;
    }
    case 99: {
      grafo.mostrarAjuda();
      solicitarEnter();
      break;
    }
    case 0: {
      cout << GREEN << "\n✅ Encerrando aplicação. Até breve!" << RESET << '\n';
      executando = false;
      break;
    }
    default: {
      cout << RED << "❌ Opção inválida! Tente novamente." << RESET << '\n';
      solicitarEnter();
      break;
    }
    }
  }

  return 0;
}
