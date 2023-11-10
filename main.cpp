#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>

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
        file << data.dump();
        file.close();
    }
private:
    string filename_;
};

class FlightSchedule {
public:
    FlightSchedule(FileHandler& flightDataHandler) : flightDataHandler_(flightDataHandler) {}

    json checkPlanes(const string& city1, const string& city2) {
        json jsonData = flightDataHandler_.loadJsonData();
        return jsonData[city1][city2];
    }

    json getFlightDetails(const string& planeId, const string& time) {
        json jsonData = flightDataHandler_.loadJsonData();
        json result;
        for (const auto& departureCity : jsonData.items()) {
            const string& departure = departureCity.key();
            for (const auto& destinationCity : departureCity.value().items()) {
                const string& destination = destinationCity.key();
                for (const auto& day : destinationCity.value().items()) {
                    const string& weekDay = day.key();
                    for (const auto& flight : day.value().items()) {
                        if (flight.key() == planeId && flight.value() == time) {
                            result["week_day"] = weekDay;
                            result["departure_city"] = departure;
                            result["destination_city"] = destination;
                        }
                    }
                }
            }
        }
        return result;
    }
private:
    FileHandler flightDataHandler_;
};

class Airplane {
public:
    Airplane(FileHandler& planeDataHandler) : planeDataHandler_(planeDataHandler) {}

    json checkSeats(const string& planeId) {
        json result;
        json planeInfo = planeDataHandler_.loadJsonData();
        result["free_seats"] = planeInfo[planeId]["free_seats"];
        for (const auto& zone : {"front", "center", "back"}) {
            json zoneInfo;
            zoneInfo["free_seats"] = planeInfo[planeId][zone]["free_seats"];
            zoneInfo["price"] = planeInfo[planeId][zone]["price"];
            result[zone] = zoneInfo;
        }
        return result;
    }

    int getPrice(const string& planeId, const string& seat) {
        json planeInfo = planeDataHandler_.loadJsonData();
        int price = 0;
        for (const auto& zone : {"front", "center", "back"}) {
            for (const auto& freeSeat : planeInfo[planeId][zone]["free_seats"]) {
                if (freeSeat == seat) {
                    price = planeInfo[planeId][zone]["price"];
                }
            }
        }
        return price;
    }

    void updateFile(const string& planeId, const string& seat) {
        json planeInfo = planeDataHandler_.loadJsonData();
        for (const auto& zone : {"front", "center", "back"}) {
            json& freeSeats = planeInfo[planeId][zone]["free_seats"];
            auto it = find(freeSeats.begin(), freeSeats.end(), seat);
            if (it != freeSeats.end()) {
                freeSeats.erase(it);
                planeInfo[planeId]["free_seats"] = planeInfo[planeId]["free_seats"].get<int>() - 1;
                planeDataHandler_.writeJsonData(planeInfo);
                break;
            }
        }
    }

private:
    FileHandler planeDataHandler_;
};

class Ticket {
public:
    Ticket(FlightSchedule& flightSchedule, Airplane& airplane) : flightSchedule_(flightSchedule), airplane_(airplane) {}

    static int generateRandomTicketId() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(10000, 99999);
        return dis(gen);
    }

    string bookSeat(const string& planeId, const string& time, const string& seat, const string& username) {
        int price = airplane_.getPrice(planeId, seat);
        if (price != 0) {
            json flightDetails = flightSchedule_.getFlightDetails(planeId, time);
            json seatsInfo = airplane_.checkSeats(planeId);
            airplane_.updateFile(planeId, seat);
            string ticketId;
            while (true) {
                ticketId = to_string(generateRandomTicketId());
                auto it = ticketInfo_.find(ticketId);
                if (it == ticketInfo_.end()) {
                    break;
                }
            }
            json ticketDetails;
            ticketDetails["cities"] = {flightDetails["departure_city"], flightDetails["destination_city"]};
            ticketDetails["date"] = {flightDetails["week_day"], time};
            ticketDetails["seatInfo"] = {planeId, seat, price};
            ticketInfo_[ticketId] = ticketDetails;
            userTickets_[username].push_back(ticketId);
            return ticketId;
        } else {
            return "Seat is taken";
        }
    }
private:
    FlightSchedule flightSchedule_;
    Airplane airplane_;
    json ticketInfo_;
    json userTickets_;
};

int main() {
    FileHandler flightDataHandler(R"(C:\Users\Admin\CLionProjects\first-oop-project\flightData.json)");
    FileHandler planeDataHandler(R"(C:\Users\Admin\CLionProjects\first-oop-project\planeData.json)");
    FlightSchedule flightSchedule(flightDataHandler);
    Airplane airplane(planeDataHandler);
    Ticket ticket(flightSchedule, airplane);
    int command;
    string city1, city2, planeId, time, seat, username;
    cout << "\n--Welcome to the Osta transportation company!--\n" << endl;
    while (true) {
        cout << "Enter 1 to check available planes/2 to check seats/3 to book a seat/4 to stop the program:" << endl;
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
            cout << "Enter planeId:" << endl;
            getline(cin, planeId);
            json seatsInfo = airplane.checkSeats(planeId);
            cout << "Seats information:" << seatsInfo << endl;
        } else if (command == 3) {
            cout << "Enter planeId:" << endl;
            getline(cin, planeId);
            cout << "Enter time:" << endl;
            getline(cin, time);
            cout << "Enter seat:" << endl;
            getline(cin, seat);
            cout << "Enter username:" << endl;
            getline(cin, username);
            string ticketId = ticket.bookSeat(planeId, time, seat, username);
            cout << "TicketId: " << ticketId << endl;
        } else if (command == 4) {
            cout << "Program stopped" << endl;
            break;
        } else {
            cout << "Enter a valid command" << endl;
        }
    }
    return 0;
}