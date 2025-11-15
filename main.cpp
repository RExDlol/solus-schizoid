#include <cstdint>
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/presence.h>
#include <dpp/snowflake.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <type_traits>
#include <vector>
#include <random>

const std::string BOT_TOKEN = std::getenv("DISCORD_TOKEN");

int main() {
    srand(std::time(nullptr));

    /* lendo o audio do carlinhos */
    /* audio vai ser o ponteiro que guarda as informações do audio (arr) */
    uint8_t *audio = nullptr;

    /* o tamanho total do audio pro arr */
    size_t audio_size = 0;

    /* carregando arquivo */
    std::ifstream input ("carlinhos2.pcm", std::ios::in|std::ios::binary|std::ios::ate);

    /* lendo e aplicando no array */
    if (input.is_open()) {

        if (!input.is_open()) {
            std::cerr << "Erro: não consegui abrir o arquivo Robot.pcm!\n";
            return 1;
        }
        else {
            std::cout << "Arquivo aberto com sucesso!\n";
        }

        audio_size = input.tellg();
        if (audio_size <= 0) {
            std::cerr << "Erro: tamanho de arquivo inválido (" << audio_size << ")\n";
            return 1;
        }
        else {
            std::cout << "Tamanho do arquivo válido!\n";
        }

        /* fala q o tamanho do arquivo é onde o cursor ta (nessa caso ele ta no final pela flag std::ios::ate) */
        audio_size = input.tellg();
        /* alocando na heap o arr do audio (sem nada) */
        audio = new uint8_t[audio_size];

        /* botando o curso no começo */
        input.seekg(0, std::ios::beg);
        input.read((char*)audio, audio_size);

        input.close();
    }

    dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content );

    bot.on_log(dpp::utility::cout_logger());

    /* carregando config dos users */
    /*
    std::ifstream file("config.json");
    nlohmann::json config;
    file >> config;




    std::ofstream o("pretty.json");
    o << std::setw(4) << config << std::endl;
    */

    /* comandos */
    bot.on_slashcommand([&bot, audio, audio_size](const dpp::slashcommand_t& event){
        dpp::emoji white_cross("_whitecross", 1436383927899127919);
        dpp::emoji baby_think("babythink", 1436383083975676026);
        dpp::emoji meowl("meowl", 1436382693280448532);
        dpp::emoji sunglas("sunglas", 1436382997963210844);

        if (event.command.get_command_name() == "seaofsorrow") {
            event.reply("essa musica é mt massa");
        }
        else if (event.command.get_command_name() == "avaliar") {
            dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("user"));

            if (user_id == event.command.get_issuing_user().id) {
                event.reply("ave mariakkkkkkkkk nao da pra avaliar a si mesmo ne " + meowl.get_mention());
                return;
            }

            if (event.command.get_issuing_user().is_bot()) {
                event.reply("voces usuarios comuns e mortais não pode avalias nós, bots superiores " + sunglas.get_mention());
                return;
            }

            std::string user_id_str = std::to_string(user_id);

            /* pega o usuário do cache */
            dpp::user* user = dpp::find_user(user_id);

            std::string username;
            if (user) {
                username = user->username;
            } else {
                username = "Desconhecido";
            }

            /* lendo json */
            std::ifstream i("config.json");
            nlohmann::json j;
            i >> j;

            /* atualizando json */
            if (!j[user_id_str].is_object()) {
                int old_value = 0;
                if (j[user_id_str].is_number_integer()) {
                    old_value = j[user_id_str].get<int>();
                }
                j[user_id_str] = nlohmann::json::object();
                j[user_id_str]["reputacao"] = old_value;
            }

            j[user_id_str]["reputacao"] = j[user_id_str].value("reputacao", 0) + 1;
            j[user_id_str]["username"] = username;


            /* salvando json */
            std::ofstream o("config.json");
            o << std::setw(4) << j << std::endl;



            dpp::embed embed = dpp::embed()
                .set_color(0xFFFFFF)
                .set_title(white_cross.get_mention() + " | Usuário avalido com sucesso!")
                .set_description("Avaliado: **" + username + "** " + sunglas.get_mention())
                .add_field("Reputação", std::to_string(j[user_id_str]["reputacao"].get<int>()) + " - " + meowl.get_mention(), true);
            dpp::message msg(event.command.channel_id, embed);

            event.reply(msg);
        }
        else if (event.command.get_command_name() == "criar_missoes"){
            std::string text = std::get<std::string>(event.get_parameter("text"));
            std::ifstream m("missoes.json");
            nlohmann::json json;
            m >> json;

            json["missoes"].push_back({
                {"id", std::to_string(event.command.usr.id)},
                {"Nome", event.command.usr.format_username()},
                {"missao", text},
            });
            std::ofstream o("missoes.json");
            o << json.dump(4);
            event.reply("Missão criada com sucesso!" + baby_think.get_mention());
        }
        else if (event.command.get_command_name() == "missao"){
            std::ifstream m("missoes.json");
            nlohmann::json json;
            m >> json;

            if(json["missoes"].empty()){
                event.reply("Não há nenhuma missão registrada, seja o primeiro a colocar uma!");
                return;
            }

            int index = rand() % json["missoes"].size();
            auto ms = json["missoes"][index];

            std::ofstream o("missoes.json");
            o << json.dump(4);

            std::string text = "**missão tirada**\n" "**Missão:**" + ms["missao"].get<std::string>();
            event.reply(text);

            bot.direct_message_create(event.command.usr.id, dpp::message(text));
            json["missoes"].erase(json["missoes"].begin() + index);
        }
        else if (event.command.get_command_name() == "reputacao") {
            dpp::snowflake user;

            if (event.get_parameter("user").index() == 0) {
                user = event.command.get_issuing_user().id;
            }
            else {
                user = std::get<dpp::snowflake>(event.get_parameter("user"));
            }
            std::string user_id_str = std::to_string(user);

            dpp::user* auser = dpp::find_user(user);

            std::string username;
            if (auser) {
                username = auser->username;
            } else {
                username = "Desconhecido";
            }

            std::ifstream i("config.json");
            nlohmann::json j;
            i >> j;

            int reputacao = 0;
            if (j.contains(user_id_str) && j[user_id_str].contains("reputacao")) {
                reputacao = j[user_id_str]["reputacao"];
            }


            dpp::embed embed = dpp::embed();
            embed.set_color(0xFFFFFF);
            embed.set_title(white_cross.get_mention() + " | Reputação do usuário!");
            embed.set_description("Usuário: **" + username + "**");
            embed.add_field("Reputação", std::to_string(reputacao) + " - " + meowl.get_mention(), true);
            event.reply(embed);
        }
        else if (event.command.get_command_name() == "top-reputacao") {
            std::ifstream i("config.json");
            nlohmann::json j;
            i >> j;

            std::vector<std::pair<std::string, int>> top;

            for (const auto& pair : j.items()) {
                // pegar o campo "reputacao" do objeto
                int reputacao = pair.value().at("reputacao").get<int>();
                top.push_back(std::make_pair(pair.key(), reputacao));
            }

            std::sort(top.begin(), top.end(), [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

            std::string top_message = "";
            for (int i = 0; i < std::min(10, static_cast<int>(top.size())); i++) {
                std::string uid = top[i].first;
                dpp::user* user = dpp::find_user(uid);
                std::string username = (user) ? user->username : uid;
                top_message += "" + sunglas.get_mention() + " | "  + std::to_string(i + 1) + ": " + username + " - " + std::to_string(top[i].second) + "\n";
            }

            dpp::embed embed;
            embed.set_title(white_cross.get_mention() + " | Top 10 usuários por reputação");
            embed.set_color(0xFFFFFF);
            embed.set_description(top_message);

            event.reply(dpp::message(event.command.channel_id, embed));
        }
        else if (event.command.get_command_name() == "entrar-call") {
            /* pegando o id do servidor */
            dpp::guild *g = dpp::find_guild(event.command.guild_id);

            /* pegando o canal de voz q nos estamos (retorna nullptr caso não, fazer try catch dps) */
            auto current_vc = event.from()->get_voice(event.command.guild_id);

            bool join_vc = true;

            /* estamos em um voice channel? se sim, estamos no correto */
            if (current_vc) {
                /* o canal que o issuer esta */
                auto vc_user = g->voice_members.find(event.command.get_issuing_user().id);

                if (vc_user != g->voice_members.end() && current_vc->channel_id == vc_user->second.channel_id) {
                    join_vc = false;

                    /* aqui pode mandar o audio */
                }
                else {
                    /* opa, nao estamos no vc, sairemos e entraremos no seu */

                    event.from()->disconnect_voice(event.command.guild_id);

                    join_vc = true;
                }
            }

            /* agora sim entraremos na call certa se precisamos */
            if (join_vc) {
                /* tenta entrar e se n der, fala iss*/
                if (!g->connect_member_voice(*event.owner, event.command.get_issuing_user().id)) {
                    event.reply(white_cross.get_mention() + " | voce nao parece estar numa call genio quiz 100 " + sunglas.get_mention());
                }


                event.reply(white_cross.get_mention() + " | entrei no seu voice channel " + sunglas.get_mention());
            }
            else {
                event.reply(white_cross.get_mention() + " | ja estou no seu voice channel seu pinguço " + sunglas.get_mention());
            }
        }
        else if (event.command.get_command_name() == "sair-call") {
            /* pegando o id do servidor */
            dpp::guild *g = dpp::find_guild(event.command.guild_id);

            /* pegando o canal de voz q o bot ta (retorna nullptr caso não, fazer try catch dps) */
            auto current_vc = event.from()->get_voice(event.command.guild_id);

            if (current_vc) {
                event.from()->disconnect_voice(event.command.guild_id);

                event.reply(white_cross.get_mention() + " | sai do seu voice channel, voces meros mortais nao merecem dividir a mesma call que eu " + sunglas.get_mention());
            }
            else {
                event.reply(white_cross.get_mention() + " | nao estou numa call visse " + sunglas.get_mention());
            }
        }
        else if (event.command.get_command_name() == "carlinhos") {
            namespace fs = std::filesystem;

            std::vector<std::string> audios;
            std::string pasta = "carlinhos/";


            for (const auto& entry : fs::directory_iterator(pasta)) {
                if (entry.path().extension() == ".pcm") {
                    audios.push_back(entry.path().string());
                }
            }

            if (audios.empty()) {
                event.reply("erro 6999999 (6 noves, fale com soren ou cruiser)");
                return;
            }

            std::srand(std::time(nullptr));
            std::string arquivo_escolhido = audios[rand() % audios.size()];

            std::ifstream input(arquivo_escolhido, std::ios::in | std::ios::binary | std::ios::ate);
            if (!input.is_open()) {
                event.reply("erro 69999999 (7 noves, fale com o soren ou cruiser)");
                return;
            }

            size_t tamanho = input.tellg();
            input.seekg(0, std::ios::beg);
            std::vector<uint8_t> buffer(tamanho);
            input.read(reinterpret_cast<char*>(buffer.data()), tamanho);
            input.close();

            dpp::voiceconn* v = event.from()->get_voice(event.command.guild_id);
            if (!v || !v->voiceclient || !v->voiceclient->is_ready()) {
                event.reply("tem algo errado com o canal de voz, entra comigo primeiro!");
                return;
            }

            v->voiceclient->send_audio_raw(reinterpret_cast<uint16_t*>(buffer.data()), tamanho);

            event.reply(white_cross.get_mention() + " | tocando " + arquivo_escolhido.substr(arquivo_escolhido.find_last_of("/\\") + 1));
        }
        else if (event.command.get_command_name() == "oraculo-medonho") {

            std::vector<std::string> sujeitos = {"o cavalo", "carlinhos", "a godot", "o gamemaker", "a unity", "a unreal", "voce", "a pepsi", "o cachorro"};
            std::vector<std::string> verbos = {"irá consumir", "vai trair", "transformará", "despertará", "matará", "infectará", "ouvirá"};
            std::vector<std::string> objetos = {"a lua", "teu wifi", "a sanidade", "a realidade", "voce", "o mundo", "o @System.32", "eu mesmo (sim eu, mesmo o bot)"};

            /* pega e ja concatena */
            std::string profecia = sujeitos[rand() % sujeitos.size()] + " " +
                                   verbos[rand() % verbos.size()] + " " +
                                   objetos[rand() % objetos.size()] + "";

            event.reply("..wiuwiuwiwu....: " + profecia);
        }

    });

    bot.on_voice_ready([audio, audio_size](const dpp::voice_ready_t& event) {


        if (event.voice_client && event.voice_client->is_ready()) {
            event.voice_client->send_audio_raw((uint16_t*)audio, audio_size);
        }
    });


    const dpp::snowflake canal_permitido = 1437545740384866314;
    std::vector<std::string> palavras;

    std::srand(std::time(0));
    bot.on_message_create([&bot, &palavras, &canal_permitido](const dpp::message_create_t& event) {
        if (event.msg.author.is_bot()) return;
        static const std::vector<std::string> bandas{
            "Sepultura",
            "Baby Metal",
            "Alice in Chains",
            "Metallica",
            "Black Sabbath",
            "Snot",
            "Slipknot",
            "The Cure",
            "Beatles",
            "Guns N' Roses",
            "Gojira",
            "The Smiths"
        };
        auto to_lower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };

        std::string conteudo = to_lower(event.msg.content);

        for (const auto& palavra : bandas) {
            if (conteudo.find(to_lower(palavra)) != std::string::npos) {
                event.reply("Eu gostar muito dessa banda visse!", false);
                break;
            }
        }
        if (conteudo.find(to_lower("Haha")) != std::string::npos) {
            event.reply("HAHAHAHA, minha mãe está morta.", false);
        } else if (conteudo.find(to_lower("Raimundos")) != std::string::npos || event.msg.content.find("raimundos") != std::string::npos) {
            event.reply("Selim...", false);
        } else if (conteudo.find(to_lower("Pia.docas")) != std::string::npos) {
            event.reply("Que engraçado viu, Pia.docas vose.", false);
        } else if (conteudo.find(to_lower("Sambary Tube")) != std::string::npos) {
            event.reply("RIQUEZA RIQUEZA", false);
        } else if (conteudo.find(to_lower("Carlinhos")) != std::string::npos) {
            event.reply("Cavalos", false);
        } else if (conteudo.find(to_lower("Cavalos")) != std::string::npos) {
            event.reply("Carlinhos", false);
        }
        else if (event.msg.content.find("furry") != std::string::npos) {
            event.reply("https://media.tenor.com/GJXXKPglZPYAAAAj/speech-bubble.gif");
        }
        else if (event.msg.content.find("bumbum guloso") != std::string::npos) {
            event.reply("bumbum guloso");
        }

        if (event.msg.channel_id != canal_permitido) return;

                int aleatorio = 1 + rand() % 4;

                if (aleatorio == 1) {
                    palavras.push_back(event.msg.content);
                } else if (aleatorio == 2) {
                    bot.message_create(dpp::message(event.msg.channel_id, "hahaha 🤪"));
                } else if (aleatorio == 3) {
                    if (!palavras.empty()) {
                        std::vector<std::string> embaralhadas = palavras;
                        std::shuffle(embaralhadas.begin(), embaralhadas.end(), std::default_random_engine(std::rand()));
                        std::string frase;
                        for (const auto& p : embaralhadas) frase += p + " ";
                        bot.message_create(dpp::message(event.msg.channel_id, frase));
                        palavras.clear();
                    }
                }

                // Quando acumular muitas, manda tudo junto de forma caótica
                if (palavras.size() >= 10) {
                    std::shuffle(palavras.begin(), palavras.end(), std::default_random_engine(std::rand()));
                    std::string frase;
                    for (const auto& p : palavras) frase += p + " ";
                    bot.message_create(dpp::message(event.msg.channel_id, frase));
                    palavras.clear();
                }
    });


    /* registrando os comandos */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("seaofsorrow", "musga boa do alice in chains", bot.me.id));
            bot.global_command_create(dpp::slashcommand("avaliar", "caso alguem tenha te ajudado, tu pode avaliar ela!!", bot.me.id).add_option(
                dpp::command_option(dpp::co_user, "user", "usuario que voce quer avaliar", true)
            ));
            bot.global_command_create(dpp::slashcommand("reputacao", "teu perfil de reputação (caso tu ajude alguem, e alguém usar o /avaliar, tu ganha um a mais)", bot.me.id).add_option(
                dpp::command_option(dpp::co_user, "user", "usuario que voce quer checar", false)
            ));
            bot.global_command_create(dpp::slashcommand("top-reputacao", "top 10 de reputação", bot.me.id));
            bot.global_command_create(dpp::slashcommand("carlinhos", "fala carlinhos na call (caso ele esteja em uma)", bot.me.id));
            bot.global_command_create(dpp::slashcommand("entrar-call", "entra na call para falar coisas bizonhas", bot.me.id));
            bot.global_command_create(dpp::slashcommand("sair-call", "sai da call que ele esta", bot.me.id));
            bot.global_command_create(dpp::slashcommand("oraculo-medonho", "da uma profecia (juro que nao eh uma frase montada aleatoriamente)", bot.me.id));
            bot.global_command_create(dpp::slashcommand("criar_missoes", "envie uma missao para alguem fazer", bot.me.id));
            bot.global_command_create(dpp::slashcommand("missao", "pegue uma missao e faca"));

            bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_listening, "⤕ 21st century schizoid man "));
        }
    });

    bot.start(dpp::st_wait);
}
