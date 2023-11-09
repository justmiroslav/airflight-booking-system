#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

class FileHandler {
public:
    FileHandler(const string& filename) : filename_(filename) {}

    json loadJsonData() {
        ifstream file(filename_);
        json jsonData;
        file >> jsonData;
        file.close();
        return jsonData;
    }
    void writeJsonData(const json& data) {
        ofstream file(filename_);
        file << data;
        file.close();
    }

private:
    string filename_;
};

class FlightSchedule {
public:
    FlightSchedule(FileHandler& flightDataHandler, FileHandler& planeDataHandler)
            : flightDataHandler_(flightDataHandler), planeDataHandler_(planeDataHandler) {}

    json checkPlanes(const string& city1, const string& city2) {
        json jsonData = flightDataHandler_.loadJsonData();
        return jsonData[city1][city2];
    }

    json planeInfo(const string& planeId) {
        json jsonData = planeDataHandler_.loadJsonData();
        return jsonData[planeId];
    }
private:
    FileHandler flightDataHandler_;
    FileHandler planeDataHandler_;
};

class Airplane {
public:
    Airplane(FileHandler& flightSchedule) : flightSchedule_(flightSchedule) {}

    json checkSeats(const string& planeId) {
        json result;
        json planeInfo = flightSchedule_.loadJsonData();
        result["free_seats"] = planeInfo[planeId]["free_seats"];
        for (const auto& zone : {"front", "center", "back"}) {
            json zoneInfo;
            zoneInfo["free_seats"] = planeInfo[planeId][zone]["free_seats"];
            zoneInfo["price"] = planeInfo[planeId][zone]["price"];
            result[zone] = zoneInfo;
        }
        return result;
    }
private:
    FileHandler flightSchedule_;
};

int main() {
    FileHandler flightDataHandler(R"(C:\Users\Admin\CLionProjects\first-oop-project\flightData.json)");
    FileHandler planeDataHandler(R"(C:\Users\Admin\CLionProjects\first-oop-project\planeData.json)");
    FlightSchedule flightSchedule(flightDataHandler, planeDataHandler);
    Airplane airplane(planeDataHandler);
    int command;
    string city1, city2, planeId, time, seat, username;
    while (true) {
        cout << "Enter 1 to check available planes/2 to check seats/3 to stop the programme:" << endl;
        cin >> command;
        cin.ignore();
        if (command == 1) {
            cout << "Available cities: Kyiv, Warsaw, Istanbul, Milan, Frankfurt" << endl;
            cout << "Enter departure city: " << endl;
            getline(cin, city1);
            cout << "Enter destination city: " << endl;
            getline(cin, city2);
            if (city1 != city2) {
                json planes = flightSchedule.checkPlanes(city1, city2);
                cout << "Available flights between " << city1 << " and " << city2 << ": " << planes << endl;
            }
        } else if (command == 2) {
            cout << "Enter plane ID: " << endl;
            getline(cin, planeId);
            json seatsInfo = airplane.checkSeats(planeId);
            cout << "Seats information: " << seatsInfo << endl;
        } else if (command == 3) {
            cout << "Program stopped" << endl;
            break;
        } else {
            cout << "Enter a valid command" << endl;
        }
    }
    return 0;
}