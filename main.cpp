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
        file << data.dump(2);
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

    string findZoneBySeat(const string& planeId, const string& seat) {
        json planeInfo = planeDataHandler_.loadJsonData();
        for (const auto& zone : {"front", "center", "back"}) {
            json& freeSeats = planeInfo[planeId][zone]["free_seats"];
            if (find(freeSeats.begin(), freeSeats.end(), seat) != freeSeats.end()) {
                return zone;
            }
        }
        return "Seat not found";
    }

    static bool seatComparator(const string& seat1, const string& seat2) {
        int num1 = stoi(seat1.substr(0, seat1.size() - 1));
        int num2 = stoi(seat2.substr(0, seat2.size() - 1));
        return num1 < num2;
    }

    void refundUpdateFile(const string& planeId, const string& zone, const string& seat) {
        json planeInfo = planeDataHandler_.loadJsonData();
        json& freeSeats = planeInfo[planeId][zone]["free_seats"];
        freeSeats.push_back(seat);
        sort(freeSeats.begin(), freeSeats.end(), seatComparator);
        planeInfo[planeId]["free_seats"] = planeInfo[planeId]["free_seats"].get<int>() + 1;
        planeDataHandler_.writeJsonData(planeInfo);
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
            string zone = airplane_.findZoneBySeat(planeId, seat);
            seatsByZone_[zone].push_back(seat);
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

    string ticketInfo(const string& ticketId, bool username) {
        json ticketDetails = ticketInfo_[ticketId];
        if (!ticketDetails.is_null()) {
            string result;
            if (username) {
                for (const auto& userTickets : userTickets_.items()) {
                    if (find(userTickets.value().begin(), userTickets.value().end(), ticketId) != userTickets.value().end()) {
                        result += "Information about ticket " + ticketId + ", bought by " + userTickets.key() + ":\n";
                    }
                }
            } else {
                result += "Information about ticket " + ticketId + ":\n";
            }
            result += "Route: " + ticketDetails["cities"][0].get<string>() + " - " + ticketDetails["cities"][1].get<string>() + ";\n";
            result += "Date: " + ticketDetails["date"][0].get<string>() + ", " + ticketDetails["date"][1].get<string>() + ";\n";
            result += "Seat Info: PlaneId - " + ticketDetails["seatInfo"][0].get<string>() + ", Place - " + ticketDetails["seatInfo"][1].get<string>() + ", Price - " + to_string(ticketDetails["seatInfo"][2].get<int>()) + "$.";
            return result;
        } else {
            return "Ticket not found";
        }
    }

    string userTickets(const string& username) {
        json ticketIds = userTickets_[username];
        if (!ticketIds.is_null()) {
            string result = "Tickets bought by " + username + ":\n\n";
            int ticketCount = ticketIds.size();
            int currentTicket = 1;
            for (const auto& ticketId : ticketIds) {
                result += ticketInfo(ticketId.get<string>(), false) + "\n";
                if (currentTicket < ticketCount) {
                    result += "\n";
                }
                currentTicket++;
            }
            return result;
        } else {
            return "No tickets found for the user";
        }
    }

    string refund(const string& ticketId) {
        if (ticketInfo_.find(ticketId) != ticketInfo_.end()) {
            string zone;
            json ticketDetails = ticketInfo_[ticketId];
            string planeId = ticketDetails["seatInfo"][0].get<string>();
            string seat = ticketDetails["seatInfo"][1].get<string>();
            string price = to_string(ticketDetails["seatInfo"][2].get<int>());
            for (const auto& zoneSeats : seatsByZone_.items()) {
                if (find(zoneSeats.value().begin(), zoneSeats.value().end(), seat) != zoneSeats.value().end()) {
                    zone = zoneSeats.key();
                    zoneSeats.value().erase(find(zoneSeats.value().begin(), zoneSeats.value().end(), seat));
                }
            }
            airplane_.refundUpdateFile(planeId, zone, seat);
            ticketInfo_.erase(ticketId);
            string username;
            for (const auto& userTickets : userTickets_.items()) {
                if (find(userTickets.value().begin(), userTickets.value().end(), ticketId) != userTickets.value().end()) {
                    username = userTickets.key();
                    userTickets.value().erase(find(userTickets.value().begin(), userTickets.value().end(), ticketId));
                    if (userTickets.value().empty()) {
                        userTickets_.erase(username);
                    }
                    break;
                }
            }
            return "Confirmed refund of " + price + "$ for " + username;
        } else {
            return "Ticket not found";
        }
    }
private:
    FlightSchedule flightSchedule_;
    Airplane airplane_;
    json ticketInfo_;
    json userTickets_;
    json seatsByZone_;
};

int main() {
    FileHandler flightDataHandler(R"(C:\Users\Admin\CLionProjects\first-oop-project\flightData.json)");
    FileHandler planeDataHandler(R"(C:\Users\Admin\CLionProjects\first-oop-project\planeData.json)");
    FlightSchedule flightSchedule(flightDataHandler);
    Airplane airplane(planeDataHandler);
    Ticket ticket(flightSchedule, airplane);
    int command;
    string city1, city2, planeId, time, seat, username, Id;
    cout << "\n--Welcome to the Osta transportation company!--\n" << endl;
    while (true) {
        cout << "1-Planes/2-Seats/3-Book seat/4-Refund/5-Ticket info/6-User tickets/7-Stop the program:" << endl;
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
            cout << "Enter Ticket ID:" << endl;
            getline(cin, Id);
            string refundDetails = ticket.refund(Id);
            cout << refundDetails << endl;
        } else if (command == 5) {
            cout << "Enter Ticket ID:" << endl;
            getline(cin, Id);
            string ticketDetails = ticket.ticketInfo(Id, true);
            cout << ticketDetails << endl;
        } else if (command == 6) {
            cout << "Enter username:" << endl;
            getline(cin, username);
            string userTicketsDetails = ticket.userTickets(username);
            cout << userTicketsDetails << endl;
        } else if (command == 7) {
            cout << "Program stopped" << endl;
            break;
        } else {
            cout << "Enter a valid command" << endl;
        }
    }
    return 0;
}