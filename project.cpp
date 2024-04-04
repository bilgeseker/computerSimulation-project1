#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>

// initialized the parameters according to given values
const int min_age_qua = 30;
const int min_age_aed = 36;
const int min_age_pra = 39;
const int min_age_con = 42;
const int yearOfService_aed = 2;
const int yearOfService_pra = 2;
const int yearOfService_con = 2;
const int reelection_interval = 10;
const int positionAvailableAnnual_qua = 20;
const int positionAvailableAnnual_aed = 10;
const int positionAvailableAnnual_pra = 8;
const int positionAvailableAnnual_con = 2;
const int simulationYear = 200;
const int startingPSI = 100;
const int unfilledPosPenalty = 5;
const int reelectingConsulPenalty = 10;

class Politician {
public:
    int age; //age of the politician
    int death_age; //age of death
    std::string office; //the position of the politician
    int years_in_office; //years of service of the politician
    Politician(int initial_age, int death_age) : age(initial_age), death_age(death_age), office("Quaestor"), years_in_office(0) {} //initialize politician

    // created a function to change the office of the politician. 
    // according to the years of service, the office will change
    void changeOffice() {
        if (office == "Quaestor") {
            office = "Aedile";
        } else if (office == "Aedile") {
            office = "Praetor";
        } else if (office == "Praetor") {
            office = "Consul";
        }
        years_in_office = 0;
    }
};

//this function finds the number of politicians in specific office.
int count_politicians(const std::vector<Politician>& politicians, const std::string& office) {
    int count = 0;
    for (const Politician& politician : politicians) {
        if (politician.office == office) {
            count++;
        }
    }
    return count;
}

int truncated_normal_distribution(double mean, double std_dev, double min_val, double max_val) {
    std::random_device rd;
    std::default_random_engine gen(rd());//create random number generator
    std::normal_distribution<> dist(mean, std_dev); //create normal distribution object
    
    int age;
    do {
        age = std::round(dist(gen));
    } while (age < min_val || age > max_val); //it will create random ages until the age falls in the range [min_val, max_val]
    
    return age;
}

std::pair<std::vector<int>, std::vector<int> > generate_initial_ages_and_death_ages(int num_politicians) {
    std::vector<int> initial_ages, death_ages;
    double initial_age_mean = 55.0, initial_age_std_dev = 10.0, initial_age_min = 25.0, initial_age_max = 50.0;
    double death_age_mean = 55.0, death_age_std_dev = 10.0, death_age_min = 25.0, death_age_max = 80.0; // Adjust these values as needed
    for (int i = 0; i < num_politicians; ++i) {
        initial_ages.push_back(truncated_normal_distribution(initial_age_mean, initial_age_std_dev, initial_age_min, initial_age_max));
        death_ages.push_back(truncated_normal_distribution(death_age_mean, death_age_std_dev, death_age_min, death_age_max));
    }
    return std::make_pair(initial_ages, death_ages);
}

void update_politician_ages(std::vector<Politician>& politicians) {
    auto it = politicians.begin();
    // check if the politian's age exceeds the death age, if it is then remove the politician from the list
    // if it is not, then increment its age
    while (it != politicians.end()) {
        if (it->age >= it->death_age) {
            it = politicians.erase(it); 
        } else {
            it->age++; 
            ++it;
        }
    }
}

std::vector<Politician> initialize_politicians(int num_politicians) {
    std::pair<std::vector<int>, std::vector<int> > result = generate_initial_ages_and_death_ages(num_politicians);
    std::vector<int>& initial_ages = result.first;
    std::vector<int>& death_ages = result.second;

    std::vector<Politician> politicians;
    for (int i = 0; i < num_politicians; ++i) {
        politicians.emplace_back(initial_ages[i], death_ages[i]);
    }
    return politicians;
}

void electionSimulation(std::vector<Politician>& politicians, int& psi) {
    for (Politician& politician : politicians) {
        // we need to check if the politician's age exceeds the death age. If it is, then skip the politician
         if (politician.age >= politician.death_age) { 
            continue;
        }
        //if the position is quaestor and the age of the politician is higher than 30, the position will change
        if (politician.office == "Quaestor" && politician.age >= min_age_qua) {
            politician.changeOffice();
        }
        // if the position is aedile and the age of the politician is higher than 36 and years of service is equal or higher than 2, the position will change
        else if (politician.office == "Aedile" && politician.age >= min_age_aed && politician.years_in_office >= yearOfService_aed) {
            politician.changeOffice();
        } 
        // if the position is praetor and the age of the politician is higher than 39 and years of service is equal or higher than 2, the position will change
        else if (politician.office == "Praetor" && politician.age >= min_age_pra && politician.years_in_office >= yearOfService_pra) {
            politician.changeOffice();
        } 
        // if the position is consul and the age of the politician is higher than 42 and years of service is equal or higher than 2
        // first check that if consul is reelected before 10 years of service or not. If it is, then apply penalty.
        else if (politician.office == "Consul" && politician.age >= min_age_con && politician.years_in_office >= yearOfService_con) {
            if (politician.years_in_office < reelection_interval) {
                psi -= reelectingConsulPenalty;
            }
            politician.changeOffice();
        }
        politician.years_in_office++; //after the election, increase the years of service of the politician
    }

    // to find if there is unfilled positions, used count_politician funtion. It counts the number of politicians in positions.
    // then this number will be subtract from position available annualy, so that it gives unfilled positions.
    // then for each unfilled position -5 PSI will be applied. Finally, these penalties are added to PSI.
    int unfill_pos_penalty = ((positionAvailableAnnual_qua - count_politicians(politicians, "Quaestor"))
                             + (positionAvailableAnnual_aed - count_politicians(politicians, "Aedile"))
                             +(positionAvailableAnnual_pra - count_politicians(politicians, "Praetor"))
                             + (positionAvailableAnnual_con - count_politicians(politicians, "Consul"))) * unfilledPosPenalty;
    psi -= unfill_pos_penalty;
}



void simulate_progression() {
    // Initialize politicians with random ages based on truncated normal distribution
    std::vector<Politician> politicians = initialize_politicians(40);
    int psi = startingPSI;

    // Simulate yearly cycle
    for (int year = 0; year < simulationYear; ++year) {
        electionSimulation(politicians, psi);
        update_politician_ages(politicians);
    }

    // Calculate end-of-simulation PSI
    std::cout << "End-of-Simulation PSI: " << psi << std::endl;

    // Calculate annual fill rates of political offices
    int total_positions_filled = 40;
    double quaestor_fill_rate = static_cast<double>(count_politicians(politicians, "Quaestor")) / total_positions_filled * 100.0;
    double aedile_fill_rate = static_cast<double>(count_politicians(politicians, "Aedile")) / total_positions_filled * 100.0;
    double praetor_fill_rate = static_cast<double>(count_politicians(politicians, "Praetor")) / total_positions_filled * 100.0;
    double consul_fill_rate = static_cast<double>(count_politicians(politicians, "Consul")) / total_positions_filled * 100.0;
    std::cout << "Annual Fill Rate:\n"
              << "Quaestor: " << quaestor_fill_rate << "%\n"
              << "Aedile: " << aedile_fill_rate << "%\n"
              << "Praetor: " << praetor_fill_rate << "%\n"
              << "Consul: " << consul_fill_rate << "%\n";

    // Calculate age distribution of politicians in different offices
    // Remaining code for age distribution calculation remains unchanged
}

int main() {
    simulate_progression();
    return 0;
}
