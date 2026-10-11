#pragma once
#include <mkw/math.hpp>
#include <array>
#include <cstdint>
#include <vector>

namespace mkw {
// Team battle rules for Balloon Battle and Coin Runners, and team scoring for
// Team VS. Pure state at 60 Hz: main supplies racer positions and the hits
// each step and draws the result. Retail's own battle code is not ported; the
// numbers here (3 balloons, 3:00, coins dropped per hit, respawn times) follow
// the game's visible behaviour.
enum class BattleKind : uint8_t {None,Balloon,Coin};
struct BattleCoin {Vec3 position{};uint16_t respawn=0;bool active=true;};

class Battle {
public:
    static constexpr unsigned MaxRacers=12;
    static constexpr unsigned StartBalloons=3,MaxBalloons=5;
    static constexpr unsigned Duration=3*60*60;        // 3:00 at 60 Hz
    static constexpr unsigned BalloonRespawn=3*60;     // out of balloons -> back with 3
    static constexpr unsigned CoinRespawn=10*60;       // a collected coin comes back
    static constexpr unsigned CoinsLostPerHit=3;
    static constexpr float CoinReach=150.f;            // kart centre to coin centre

    // count racers; racer 0 is the local player, on team 0 (red).
    void start(BattleKind kind,unsigned count,std::vector<Vec3> coinSpots={});
    // Team VS: teams only (no timer, balloons or coins).
    void startTeams(unsigned count);
    void stop(){kind_=BattleKind::None;teams_=false;}
    bool battle() const {return kind_!=BattleKind::None;}
    bool teams() const {return teams_;}
    BattleKind kind() const {return kind_;}

    // One 60 Hz step of a running battle. hits: racers struck this step (spin,
    // tumble, blast, crush...). Returns true on the step the timer runs out.
    bool step(const std::array<Vec3,MaxRacers>& positions,const std::array<bool,MaxRacers>& hits);

    int8_t team(unsigned racer) const {return racer<count_?team_[racer]:-1;}
    unsigned balloons(unsigned racer) const {return racer<count_?balloons_[racer]:0;}
    unsigned coins(unsigned racer) const {return racer<count_?coins_[racer]:0;}
    // Points scored by this racer (balloons popped, or coins held).
    unsigned personal(unsigned racer) const {return racer<count_?(kind_==BattleKind::Coin?coins_[racer]:pops_[racer]):0;}
    bool out(unsigned racer) const {return racer<count_&&respawn_[racer]>0;}
    unsigned score(unsigned t) const;
    unsigned framesLeft() const {return left_;}
    bool over() const {return battle()&&left_==0;}
    // 0 red, 1 blue, -1 draw.
    int winner() const;
    const std::vector<BattleCoin>& coinSpots() const {return spots_;}
    unsigned count() const {return count_;}

private:
    BattleKind kind_=BattleKind::None;bool teams_=false;
    unsigned count_=0,left_=0;
    std::array<int8_t,MaxRacers> team_{};
    std::array<uint8_t,MaxRacers> balloons_{};
    std::array<uint16_t,MaxRacers> coins_{},pops_{},respawn_{};
    std::array<unsigned,2> teamPops_{};
    std::vector<BattleCoin> spots_;
};
}
