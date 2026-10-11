#include <mkw/battle.hpp>
#include <algorithm>
#include <limits>

namespace mkw {
namespace {
// Red takes the even grid slots, blue the odd ones, so both teams start mixed
// as on the retail grid; racer 0 (the player) is red.
int8_t teamOf(unsigned racer){return int8_t(racer&1);}
}

void Battle::start(BattleKind kind,unsigned count,std::vector<Vec3> coinSpots){
    kind_=kind;teams_=true;count_=std::min(count,MaxRacers);left_=Duration;
    team_={};balloons_={};coins_={};pops_={};respawn_={};teamPops_={};spots_.clear();
    for(unsigned i=0;i<count_;++i){team_[i]=teamOf(i);balloons_[i]=kind==BattleKind::Balloon?StartBalloons:0;}
    if(kind==BattleKind::Coin)for(const auto& p:coinSpots)spots_.push_back({p,0,true});
}

void Battle::startTeams(unsigned count){
    kind_=BattleKind::None;teams_=true;count_=std::min(count,MaxRacers);left_=0;
    team_={};balloons_={};coins_={};pops_={};respawn_={};teamPops_={};spots_.clear();
    for(unsigned i=0;i<count_;++i)team_[i]=teamOf(i);
}

bool Battle::step(const std::array<Vec3,MaxRacers>& positions,const std::array<bool,MaxRacers>& hits){
    if(!battle()||!left_)return false;
    for(unsigned i=0;i<count_;++i){
        if(respawn_[i]&&!--respawn_[i]&&kind_==BattleKind::Balloon)balloons_[i]=StartBalloons;
        if(!hits[i]||respawn_[i])continue;
        if(kind_==BattleKind::Balloon){
            if(!balloons_[i])continue;
            --balloons_[i];
            const unsigned other=unsigned(1-team_[i]);++teamPops_[other];
            // Retail credits the attacker; item hits here only name the victim, so the
            // nearest opponent within reach takes the personal point.
            unsigned credit=MaxRacers;float best=3000.f*3000.f;
            for(unsigned j=0;j<count_;++j)if(team_[j]!=team_[i]){const Vec3 d=positions[j]-positions[i];const float d2=dot(d,d);if(d2<best){best=d2;credit=j;}}
            if(credit<count_)++pops_[credit];
            if(!balloons_[i])respawn_[i]=uint16_t(BalloonRespawn);
        }else{
            coins_[i]=uint16_t(coins_[i]>CoinsLostPerHit?coins_[i]-CoinsLostPerHit:0);
        }
    }
    if(kind_==BattleKind::Coin){
        for(auto& c:spots_){
            if(!c.active){if(c.respawn&&!--c.respawn)c.active=true;continue;}
            for(unsigned i=0;i<count_;++i){
                const Vec3 d=positions[i]-c.position;
                if(dot(d,d)<CoinReach*CoinReach){++coins_[i];c.active=false;c.respawn=uint16_t(CoinRespawn);break;}
            }
        }
    }
    return --left_==0;
}

unsigned Battle::score(unsigned t) const {
    if(t>1)return 0;
    if(kind_==BattleKind::Balloon)return teamPops_[t];
    unsigned sum=0;for(unsigned i=0;i<count_;++i)if(unsigned(team_[i])==t)sum+=kind_==BattleKind::Coin?coins_[i]:0;
    return sum;
}

int Battle::winner() const {
    const unsigned r=score(0),b=score(1);
    return r>b?0:b>r?1:-1;
}
}
