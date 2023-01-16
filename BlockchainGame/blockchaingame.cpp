#include <inery/inery.hpp>
#include <string>


using namespace inery;
using namespace std;

class [[inery::contract("inerygame")]] store : public contract {
    public:
        using contract::contract;

        struct [[inery::table("scores")]] scores 
        {
            uint64_t    id;
            string      username;
            uint64_t    score;
            uint64_t primary_key() const { return id; }
        };
        typedef inery::multi_index<"scores"_n, scores> scores_inst;

        struct [[inery::table("totalscores")]] totalscores 
        {
            uint64_t    id;
            string      username;
            uint64_t    score;
            uint64_t primary_key() const { return id; }
        };
        typedef inery::multi_index<"totalscores"_n, totalscores> total_scores_inst;



        [[inery::action]] void insertscore(string username, uint64_t score){
            require_auth("inerygame"_n);

            vector < scores > temp_storage;
            
            scores_inst scores_table(get_self(), name("inerygame").value);
            total_scores_inst total_scores_table(get_self(), name("inerygame").value);

            scores_table.emplace(name("inerygame"), [&](auto &player){
                player.id       = scores_table.available_primary_key();
                player.username = username;
                player.score    = score;
            });

            total_scores_table.emplace(name("inerygame"), [&](auto &player){
                player.id       = total_scores_table.available_primary_key();
                player.username = username;
                player.score    = score;
            });

            uint64_t dist = distance(scores_table.begin(), scores_table.end());
            temp_storage.reserve(dist);

            for(auto scores_itr = scores_table.begin(); scores_itr != scores_table.end() && temp_storage.size() < dist; scores_itr++){
                temp_storage.emplace_back(scores{
                    .id = scores_itr->id,
                    .username = scores_itr->username,
                    .score = scores_itr->score
                });
            }

            sort(temp_storage.begin(), temp_storage.end(), [](const scores& left, const scores& right){
                return left.score > right.score;
            });

            for(auto scores_itr = temp_storage.begin(); scores_itr != temp_storage.end(); scores_itr++){
                auto rmv_it = scores_table.find(scores_itr->id);
                scores_table.erase(rmv_it);
            }

            uint16_t num = 0;

            for(auto tmp_itr = temp_storage.begin(); tmp_itr != temp_storage.end() && num <= 10; tmp_itr++){
                scores_table.emplace(name("inerygame"), [&](auto &player){
                    player.id       = scores_table.available_primary_key();
                    player.username = tmp_itr->username;
                    player.score    = tmp_itr->score;
                });
                num++;
            }
        }

        [[inery::action]] void clearscores(){
            require_auth("inerygame"_n);

            scores_inst scores_table(get_self(), name("inerygame").value);
            vector< scores > rmv_scores;
            uint64_t dist = distance(scores_table.begin(), scores_table.end());
            rmv_scores.reserve(dist);
            
            for(auto it = scores_table.begin(); it != scores_table.end(); it++)
                rmv_scores.emplace_back(scores{
                    .id = it->id,
                    .username = it->username,
                    .score = it->score
                });

            for(auto it = rmv_scores.begin(); it != rmv_scores.end(); it++){
                auto rmv_it = scores_table.find(it->id);
                if(rmv_it != scores_table.end())
                    scores_table.erase(rmv_it);
            }

            total_scores_inst total_scores_table(get_self(), name("inerygame").value);
            vector< scores > rmv_total;
            dist = distance(total_scores_table.begin(), total_scores_table.end());
            rmv_total.reserve(dist);
            
            for(auto it = total_scores_table.begin(); it != total_scores_table.end(); it++)
                rmv_total.emplace_back(scores{
                    .id = it->id,
                    .username = it->username,
                    .score = it->score
                });

            for(auto it = rmv_total.begin(); it != rmv_total.end(); it++){
                auto rmv_it = total_scores_table.find(it->id);
                if(rmv_it != total_scores_table.end())
                    total_scores_table.erase(rmv_it);
            }
                
        }   
};