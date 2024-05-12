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

struct raw_event {
    char date[8];
    char event_type;
    char space;
    char targets[3];
};

enum class event_type {
    SINGLES,
    DOUBLES,
    HANDICAP
};

struct event {
    char date[9];
    event_type event_type;
    unsigned short targets;
};

struct raw_header {
    char club_number[6];
    raw_event events[24];
};

struct header {
    char club_number[7];
    std::vector<event*> events;

    ~header() {
        for(auto & event : events)
            delete event;
    }
};
auto constexpr EVENTS_OFFSET = offsetof(header, events);

struct raw_score {
    char hit[3];
    char shot_at[3];
    char yardage[4];
};

struct score {
    unsigned short hit;
    unsigned short shot_at;
    float yardage; // probably would cause rounding errors, but meh
};

struct raw_shooter {
    char ata_number[7];
    char name[18];
    char address[25];
    char city[18];
    char state[2];
    char postal_code[5];
    char classification;
    raw_score scores[24];
};

struct shooter {
    char ata_number[8]; // left as a char because of possible new member numbers
    char name[19];
    char address[26];
    char city[19];
    char state[3];
    char postal_code[6];
    char classification[2];
    std::vector<score*> scores;

    ~shooter() {
        for(auto & score : scores)
            delete score;
    }
};
auto constexpr SHOOTER_SCORES_OFFSET = offsetof(shooter, scores);

struct ata_data_file {
    header header;
    std::vector<shooter*> shooters;

    ~ata_data_file() {
        for(auto & shooter : shooters)
            delete shooter;
    }
};

auto read_ata_data(const char* file_name) -> ata_data_file* {
    auto data_file = new ata_data_file;

    std::ifstream atadata(file_name, std::ios::binary);
    char line_end[2];

    raw_header raw_header{};

    atadata.read(reinterpret_cast<char*>(&raw_header), 318);
    atadata.read(line_end, 2);
    if(line_end[0] != '\r' || line_end[1] != '\n') {
        std::cerr << "Unexpected file format!" << std::endl;
        atadata.close();
        delete data_file;
        return nullptr;
    }

    memset(&data_file->header, 0, EVENTS_OFFSET);
    memcpy_s(data_file->header.club_number, 6, raw_header.club_number, 6);
    for(auto & raw_event : raw_header.events) {
        if(raw_event.event_type != 'S' && raw_event.event_type != 'D' && raw_event.event_type != 'H') break; // no more events
        auto new_event = new event;
        memset(new_event, 0, sizeof(event));

        memcpy_s(new_event->date, 8, raw_event.date, 8);
        char target_nullterm[4];
        target_nullterm[3] = '\0';
        memcpy_s(target_nullterm, 3, raw_event.targets, 3);
        new_event->targets = static_cast<unsigned short>(strtol(target_nullterm, nullptr, 10));

        if(raw_event.event_type == 'S') {
            new_event->event_type = event_type::SINGLES;
        }
        if(raw_event.event_type == 'D') {
            new_event->event_type = event_type::DOUBLES;
        }
        if(raw_event.event_type == 'H') {
            new_event->event_type = event_type::HANDICAP;
        }
        data_file->header.events.push_back(new_event);
    }

    while(!atadata.eof()) {
        auto raw_shooter = new struct raw_shooter;
        atadata.read(reinterpret_cast<char*>(raw_shooter), 316);
        atadata.read(line_end, 2);
        if(line_end[0] != '\r' || line_end[1] != '\n') {
            std::cerr << "Unexpected file format!" << std::endl;
            atadata.close();
            delete data_file;
            return nullptr;
        }

        auto shooter = new struct shooter;
        memset(shooter, 0, SHOOTER_SCORES_OFFSET);
        memcpy_s(shooter->ata_number, 7, raw_shooter->ata_number, 7);
        memcpy_s(shooter->name, 18, raw_shooter->name, 18);
        memcpy_s(shooter->address, 25, raw_shooter->address, 25);
        memcpy_s(shooter->city, 18, raw_shooter->city, 18);
        memcpy_s(shooter->state, 2, raw_shooter->state, 2);
        memcpy_s(shooter->postal_code, 5, raw_shooter->postal_code, 5);
        memcpy_s(shooter->classification, 1, &raw_shooter->classification, 1);

        for(auto & raw_score : raw_shooter->scores) {
            auto score = new struct score;
            char target_nullterm[5] = {};
            memcpy_s(target_nullterm, 3, raw_score.hit, 3);
            score->hit = static_cast<unsigned short>(strtol(target_nullterm, nullptr, 10));
            memcpy_s(target_nullterm, 3, raw_score.shot_at,3 );
            score->shot_at = static_cast<unsigned short>(strtol(target_nullterm, nullptr, 10));
            memcpy_s(target_nullterm, 4, raw_score.yardage, 4);
            score->yardage = strtof(target_nullterm, nullptr);

            shooter->scores.push_back(score);
        }
        delete raw_shooter;
        data_file->shooters.push_back(shooter);
    }
    atadata.close();
    return data_file;
}

auto print_event(const event* event) -> void {
    auto const date = event->date;
    std::cout << date[0] << date[1] << "/" << date[2] << date[3] << "/" << date[4] << date[5] << date[6] << date[7];
    std::cout << " : " << event->targets << " ";
    switch(event->event_type) {
        case event_type::SINGLES:
            std::cout << "Singles";
        break;
        case event_type::DOUBLES:
            std::cout << "Doubles";
        break;
        case event_type::HANDICAP:
            std::cout << "Handicap";
        break;
    }
}

auto display_events(const ata_data_file* ata_data) -> void {
    for(auto i=0; i<ata_data->header.events.size(); ++i) {
        auto const event = ata_data->header.events[i];
        std::cout << "\t[" << i << "] " << event->targets << " ";
        print_event(event);
        std::cout << std::endl;
    }
}

auto print_scores(const ata_data_file* ata_data) -> void {
    display_events(ata_data);
    std::cout << "Which event?" << std::endl << "Event #: ";
    char event_number[4096] = {};
    std::cin.getline(event_number, 4096);

    auto event_index = strtol(event_number, nullptr, 10);
    if(event_index < 0 || event_index >= ata_data->header.events.size()) {
        std::cout << "Invalid event selected!" << std::endl;
        print_scores(ata_data);
        return;
    }

    auto selected_event = ata_data->header.events[event_index];
    print_event(selected_event);
    std::cout << std::endl;

    auto score_map = std::multimap<unsigned short, char*, std::greater<unsigned short>>();
    for(auto & shooter : ata_data->shooters) {
        auto selected_score = shooter->scores[event_index];
        if(selected_score->shot_at == 0) continue;

        score_map.emplace(selected_score->hit, shooter->name);
    }

    std::cout << score_map.size() << " shooters" << std::endl;
    for(auto & sorted_score : score_map) {
        printf("%3d %s\n", sorted_score.first, sorted_score.second);
    }
    std::cout << std::endl;
}

auto print_shooter_info(const ata_data_file* ata_data, const shooter* shooter) -> void {
    std::cout << shooter->name << std::endl;
    std::cout << "ATA Number: " << shooter->ata_number << std::endl;
    std::cout << "Address: " << shooter->address << ", " << shooter->city << ", " << shooter->state << " " << shooter->postal_code << std::endl;
    std::cout << "Scores: " << std::endl;

    for(auto i=0; i<shooter->scores.size(); ++i) {
        if(i >= ata_data->header.events.size()) break;
        auto const score = shooter->scores[i];
        auto const event = ata_data->header.events[i];
        if(score->shot_at == 0) continue;
        std::cout << "\t";
        print_event(event);
        std::cout << "   " << score->hit << "/" << score->shot_at;
        if(score->yardage > 0.0f) {
            printf(" %1.1f yd", score->yardage);
        }
        std::cout << std::endl;
    }
}

auto display_shooters(const ata_data_file* ata_data) -> void {
    for(auto i=0; i<ata_data->shooters.size(); ++i) {
        auto const shooter = ata_data->shooters[i];
        std::cout << "\t[" << i << "] " << shooter->name << std::endl;
    }
}

auto view_shooter_info(const ata_data_file* ata_data) -> void {
    display_shooters(ata_data);
    std::cout << "Which shooter?" << std::endl << "Shooter #: ";
    char shooter_number[4096] = {};
    std::cin.getline(shooter_number, 4096);

    auto shooter_index = strtol(shooter_number, nullptr, 10);
    if(shooter_index < 0 || shooter_index >= ata_data->shooters.size()) {
        std::cout << "Invalid shooter selected!" << std::endl;
        view_shooter_info(ata_data);
        return;
    }

    auto selected_shooter = ata_data->shooters[shooter_index];
    print_shooter_info(ata_data, selected_shooter);
}

auto main(int argc, char **argv) -> int {
    if (argc != 2) {
        std::cerr << "Usage: <file>" << std::endl;
        return -1;
    }

    auto file_name = argv[1];
    auto ata_data = read_ata_data(file_name);
    if(!ata_data) {
        return -1;
    }

    std::cout << "Club Number: " << ata_data->header.club_number << std::endl;
    std::cout << ata_data->header.events.size() << " events and " << ata_data->shooters.size() << " shooters loaded.";
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
                goto quit; // GASP I DID IT
            case 'V':
            case 'v':
                view_shooter_info(ata_data);
                break;
            default:
                std::cout << "Unknown command!" << std::endl;
                break;
        }
    }

    quit:
    delete ata_data;
    return 0;
}
