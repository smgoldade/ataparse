/**
 * @file
 * @brief 
 *
 * @author Steven Goldade
 * @date 5/11/2024
 */
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <vector>

enum class event_type {
    SINGLES,
    DOUBLES,
    HANDICAP,
    NO_EVENT
};

struct event {
    char date[9];
    event_type event_type;
    unsigned short targets;

    explicit event(const char* raw_data) noexcept : date{}, event_type(event_type::NO_EVENT), targets(0) {
        memcpy_s(date, 8, raw_data, 8);

        switch(raw_data[8]) {
            case 'S':
                event_type = event_type::SINGLES;
                break;
            case 'D':
                event_type = event_type::DOUBLES;
                break;
            case 'H':
                event_type = event_type::HANDICAP;
                break;
            default:
                event_type = event_type::NO_EVENT;
                break;
        }

        if(event_type == event_type::NO_EVENT) return;

        char buffer[4]{};
        memcpy_s(buffer, 3, raw_data + 10, 3);
        targets = static_cast<unsigned short>(strtol(buffer, nullptr, 10));
    }

    event(const event& other) : date{}, event_type(other.event_type), targets(other.targets) { memcpy_s(date, 8, other.date, 8); }

    auto operator =(const event& other) -> event& {
        if(&other == this) return *this;

        memcpy_s(date, 8, other.date, 8);
        event_type = other.event_type;
        targets = other.targets;
        return *this;
    }

    event(event&& other) noexcept : date{}, event_type(other.event_type), targets(other.targets) { memcpy_s(date, 8, other.date, 8); }

    auto operator =(event&& other) = delete;
};

struct header {
    char club_number[7];
    std::vector<event> events;

    explicit header(const char* raw_data) noexcept : club_number{} {
        memcpy_s(club_number, 6, raw_data, 6);
        for(auto event_index = 0; event_index < 24; ++event_index) {
            auto const event_start = 6 + event_index * 13;
            auto const new_event = event(raw_data + event_start);

            if(new_event.event_type != event_type::NO_EVENT) events.push_back(new_event);
        }
    }

    header(const header& other) : club_number{}, events(other.events) { memcpy_s(club_number, 6, other.club_number, 6); }

    auto operator =(const header& other) -> header& {
        if(&other == this) return *this;

        memcpy_s(club_number, 6, other.club_number, 6);
        events = other.events;
        return *this;
    }

    header(header&& other) = delete;

    auto operator =(header&& other) = delete;
};

struct score {
    unsigned short hit;
    unsigned short shot_at;
    float yardage;// probably would cause rounding errors, but meh

    explicit score(const char* raw_data) : hit(0), shot_at(0), yardage(0.0f) {
        char buffer[5]{};
        memcpy_s(buffer, 3, raw_data, 3);
        hit = static_cast<unsigned short>(strtol(buffer, nullptr, 10));
        memcpy_s(buffer, 3, raw_data + 3, 3);
        shot_at = static_cast<unsigned short>(strtol(buffer, nullptr, 10));
        memcpy_s(buffer, 4, raw_data + 6, 4);
        yardage = strtof(buffer, nullptr);
    }

    score(const score& other) = default;

    auto operator=(const score& other) -> score& = default;

    score(score&& other) = default;

    auto operator=(score&& other) -> score& = default;
};

struct shooter {
    char ata_number[8];// left as a char because of possible new member numbers
    char name[19];
    char address[26];
    char city[19];
    char state[3];
    char postal_code[6];
    char classification[2];
    std::vector<score> scores;

    explicit shooter(const char* raw_data) : ata_number{}, name{}, address{}, city{}, state{}, postal_code{}, classification{} {
        memcpy_s(ata_number, 7, raw_data, 7);
        memcpy_s(name, 18, raw_data + 7, 18);
        memcpy_s(address, 25, raw_data + 25, 25);
        memcpy_s(city, 18, raw_data + 50, 18);
        memcpy_s(state, 2, raw_data + 68, 2);
        memcpy_s(postal_code, 5, raw_data + 70, 5);
        memcpy_s(classification, 1, raw_data + 75, 1);

        for(auto score_index = 0; score_index < 24; ++score_index) {
            auto const score_start = 76 + score_index * 10;
            scores.emplace_back(raw_data + score_start);
        }
    }

    shooter(const shooter& other) : ata_number{}, name{}, address{}, city{}, state{}, postal_code{}, classification{}, scores(other.scores) {
        memcpy_s(ata_number, 7, other.ata_number, 7);
        memcpy_s(name, 18, other.name, 18);
        memcpy_s(address, 25, other.address, 25);
        memcpy_s(city, 18, other.city, 18);
        memcpy_s(state, 2, other.state, 2);
        memcpy_s(postal_code, 5, other.postal_code, 5);
        memcpy_s(classification, 1, other.classification, 1);
    }

    auto operator =(const shooter& other) -> shooter& {
        if(&other == this) return *this;

        memcpy_s(ata_number, 7, other.ata_number, 7);
        memcpy_s(name, 18, other.name, 18);
        memcpy_s(address, 25, other.address, 25);
        memcpy_s(city, 18, other.city, 18);
        memcpy_s(state, 2, other.state, 2);
        memcpy_s(postal_code, 5, other.postal_code, 5);
        memcpy_s(classification, 1, other.classification, 1);
        scores = other.scores;
        return *this;
    }

    shooter(shooter&& other) noexcept : ata_number{}, name{}, address{}, city{}, state{}, postal_code{}, classification{}, scores(std::move(other.scores)) {
        memcpy_s(ata_number, 7, other.ata_number, 7);
        memcpy_s(name, 18, other.name, 18);
        memcpy_s(address, 25, other.address, 25);
        memcpy_s(city, 18, other.city, 18);
        memcpy_s(state, 2, other.state, 2);
        memcpy_s(postal_code, 5, other.postal_code, 5);
        memcpy_s(classification, 1, other.classification, 1);
    }

    auto operator=(shooter&& other) -> shooter& = delete;
};

struct ata_data_file {
    header header;
    std::vector<shooter> shooters;

    ata_data_file(const struct header& header, const std::vector<shooter>& shooters) : header(header), shooters(shooters) {}
};

auto read_ata_data(const char* file_name) -> ata_data_file {
    std::ifstream atadata(file_name, std::ios::binary);

    char line[320];
    atadata.read(line, 320);
    if(line[318] != '\r' || line[319] != '\n') {
        atadata.close();
        throw std::exception("Unexpected file format!");
    }
    header ata_data_header{line};

    std::vector<shooter> shooters;
    while(!atadata.eof()) {
        atadata.read(line, 318);
        if(line[316] != '\r' || line[317] != '\n') {
            atadata.close();
            throw std::exception("Unexpected file format!");
        }
        shooters.emplace_back(line);
    }
    atadata.close();
    return {ata_data_header, shooters};
}

auto print_event(const event& event) -> void {
    auto const date = event.date;
    std::cout << date[0] << date[1] << "/" << date[2] << date[3] << "/" << date[4] << date[5] << date[6] << date[7];
    std::cout << " : " << event.targets << " ";
    switch(event.event_type) {
        case event_type::SINGLES:
            std::cout << "Singles";
            break;
        case event_type::DOUBLES:
            std::cout << "Doubles";
            break;
        case event_type::HANDICAP:
            std::cout << "Handicap";
            break;
        default:
            break;
    }
}

auto display_events(const ata_data_file& ata_data) -> void {
    for(auto i = 0; i < ata_data.header.events.size(); ++i) {
        auto const event = ata_data.header.events[i];
        std::cout << "\t[" << i << "] ";
        print_event(event);
        std::cout << std::endl;
    }
}

auto print_scores(const ata_data_file& ata_data) -> void { // NOLINT(*-no-recursion) intentional for invalid event
    display_events(ata_data);
    std::cout << "Which event?" << std::endl << "Event #: ";
    char event_number[4096] = {};
    std::cin.getline(event_number, 4096);

    auto const event_index = strtol(event_number, nullptr, 10);
    if(event_index < 0 || event_index >= ata_data.header.events.size()) {
        std::cout << "Invalid event selected!" << std::endl;
        print_scores(ata_data);
        return;
    }

    auto const selected_event = ata_data.header.events[event_index];
    print_event(selected_event);
    std::cout << std::endl;

    auto score_map = std::multimap<unsigned short, const char*, std::greater<unsigned short> >();
    for(auto& shooter : ata_data.shooters) {
        auto selected_score = shooter.scores[event_index];
        if(selected_score.shot_at == 0) continue;

        score_map.emplace(selected_score.hit, shooter.name);
    }

    std::cout << score_map.size() << " shooters" << std::endl;
    for(auto& sorted_score : score_map) { printf("%3d %s\n", sorted_score.first, sorted_score.second); }
    std::cout << std::endl;
}

auto print_shooter_info(const ata_data_file& ata_data, const shooter& shooter) -> void {
    std::cout << shooter.name << std::endl;
    std::cout << "ATA Number: " << shooter.ata_number << std::endl;
    std::cout << "Address: " << shooter.address << ", " << shooter.city << ", " << shooter.state << " " << shooter.
        postal_code << std::endl;
    std::cout << "Scores: " << std::endl;

    for(auto i = 0; i < shooter.scores.size(); ++i) {
        if(i >= ata_data.header.events.size()) break;
        auto const score = shooter.scores[i];
        auto const event = ata_data.header.events[i];
        if(score.shot_at == 0) continue;
        std::cout << "\t";
        print_event(event);
        std::cout << "   " << score.hit << "/" << score.shot_at;
        if(score.yardage > 0.0f) { printf(" %1.1f yd", score.yardage); }
        std::cout << std::endl;
    }
}

auto display_shooters(const ata_data_file& ata_data) -> void {
    for(auto i = 0; i < ata_data.shooters.size(); ++i) {
        auto const shooter = ata_data.shooters[i];
        std::cout << "\t[" << i << "] " << shooter.name << std::endl;
    }
}

auto view_shooter_info(const ata_data_file& ata_data) -> void { // NOLINT(*-no-recursion) intentional for invalid shooter
    display_shooters(ata_data);
    std::cout << "Which shooter?" << std::endl << "Shooter #: ";
    char shooter_number[4096] = {};
    std::cin.getline(shooter_number, 4096);

    auto const shooter_index = strtol(shooter_number, nullptr, 10);
    if(shooter_index < 0 || shooter_index >= ata_data.shooters.size()) {
        std::cout << "Invalid shooter selected!" << std::endl;
        view_shooter_info(ata_data);
        return;
    }

    auto const selected_shooter = ata_data.shooters[shooter_index];
    print_shooter_info(ata_data, selected_shooter);
}

auto main(const int argc, char** argv) -> int {
    if(argc != 2) {
        std::cerr << "Usage: <file>" << std::endl;
        return -1;
    }

    auto const file_name = argv[1];
    try {
        auto const ata_data = read_ata_data(file_name);

        std::cout << "Club Number: " << ata_data.header.club_number << std::endl;
        std::cout << ata_data.header.events.size() << " events and " << ata_data.shooters.size() << " shooters loaded.";
        std::cout << std::endl;

        while(true) {
            std::cout << "[P]rint Scores, [V]iew Shooter Info, [Q]uit" << std::endl;
            std::cout << "Command: ";
            char command[256];
            std::cin.getline(command, 256);

            switch(command[0]) {
                case 'P':
                case 'p':
                    print_scores(ata_data);
                    break;
                case 'Q':
                case 'q':
                    return 0;
                case 'V':
                case 'v':
                    view_shooter_info(ata_data);
                    break;
                default:
                    std::cout << "Unknown command!" << std::endl;
                    break;
            }
        }
    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}