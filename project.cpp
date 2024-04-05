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
const int totalNumOfPoliticians = 40;

class Politician {
public:
    int age; //age of the politician
    int death_age; //age of death
    std::string position; //the position of the politician
    int yearsOfService; //years of service of the politician
    Politician(int initial_age, int death_age) : age(initial_age), death_age(death_age), position("Quaestor"), yearsOfService(0) {} //initialize politician

    // created a function to change the office of the politician. 
    // according to the years of service, the office will change
    void changeOffice() {
        if (position == "Quaestor") {
            position = "Aedile";
        } else if (position == "Aedile") {
            position = "Praetor";
        } else if (position == "Praetor") {
            position = "Consul";
        }
        yearsOfService = 0;
    }
};

//this function finds the number of politicians in specific office.
int count_politicians(const std::vector<Politician>& politicians, const std::string& position) {
    int count = 0;
     for (size_t i = 0; i < politicians.size(); ++i) {
        if (politicians[i].position == position) {
            count++;
        }
    }
    return count;
}

int truncated_normal_distribution(double mean, double std, double min_val, double max_val) {
    std::random_device rd;
    std::default_random_engine gen(rd());//create random number generator
    std::normal_distribution<> dist(mean, std); //create normal distribution object
    
    int age;
    do {
        age = std::round(dist(gen));
    } while (age < min_val || age > max_val); //it will create random ages until the age falls in the range [min_val, max_val]
    
    return age;
}

// It will be used to randomly generate the initial and death ages.
std::pair<std::vector<int>, std::vector<int> > generate_initial_death_ages(int num_politicians) {
    std::vector<int> initial_ages, death_ages; //set a vector to hold initial and death ages
    double initAgeMean = 36.0, initAgeStd = 3.0, initAgeMin = 30.0, initAgeMax = 42.0; // random initial age, min 30 and max 42
    double deathAgeMean = 55.0, deathAgeStd = 10.0, deathAgeMin = 25.0, deathAgeMax = 80.0; // random death age
    for (int i = 0; i < num_politicians; ++i) {
        int initial_age = truncated_normal_distribution(initAgeMean, initAgeStd, initAgeMin, initAgeMax); //set the initial age
        int death_age;
        do {
            death_age = truncated_normal_distribution(deathAgeMean, deathAgeStd, deathAgeMin, deathAgeMax); // set death age
        } while (death_age < initial_age); // check death age is not smaller than initial age
        initial_ages.push_back(initial_age);
        death_ages.push_back(death_age);
    }
    return std::make_pair(initial_ages, death_ages); // return the initial and death age vectors
}

void update_politician_ages(std::vector<Politician>& politicians) {
    std::vector<Politician>::iterator it = politicians.begin();
    // check if the politian's age exceeds the death age. if it is, then remove the politician from the list
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

// initialize the politicans. get the age vectors and push them into the vector.
// in Politician class, we already set the office of the politician
std::vector<Politician> initialize_politicians(int num_politicians) {
    std::pair<std::vector<int>, std::vector<int> > result = generate_initial_death_ages(num_politicians);
    std::vector<int>& initial_ages = result.first;
    std::vector<int>& death_ages = result.second;

    std::vector<Politician> politicians;
    for (int i = 0; i < num_politicians; ++i) {
        Politician politician(initial_ages[i], death_ages[i]);
        politicians.push_back(politician);
    }
    return politicians;
}

void electionSimulation(std::vector<Politician>& politicians, int& psi) {
    for (size_t i = 0; i < politicians.size(); ++i) {
        // we need to check if the politician's age exceeds the death age. If it is, then skip the politician
         if (politicians[i].age >= politicians[i].death_age) { 
            continue;
        }
        //if the position is quaestor and the age of the politician is higher than 30, the position will change
        if (politicians[i].position == "Quaestor" && politicians[i].age >= min_age_qua) {
            politicians[i].changeOffice();
        }
        // if the position is aedile and the age of the politician is higher than 36 and years of service is equal or higher than 2, the position will change
        else if (politicians[i].position == "Aedile" && politicians[i].age >= min_age_aed && politicians[i].yearsOfService >= yearOfService_aed) {
            politicians[i].changeOffice();
        } 
        // if the position is praetor and the age of the politician is higher than 39 and years of service is equal or higher than 2, the position will change
        else if (politicians[i].position == "Praetor" && politicians[i].age >= min_age_pra && politicians[i].yearsOfService >= yearOfService_pra) {
            politicians[i].changeOffice();
        } 
        // if the position is consul and the age of the politician is higher than 42 and years of service is equal or higher than 2
        // first check that if consul is reelected before 10 years of service or not. If it is, then apply penalty.
        else if (politicians[i].position == "Consul" && politicians[i].age >= min_age_con && politicians[i].yearsOfService >= yearOfService_con) {
            if (politicians[i].yearsOfService < reelection_interval) {
                psi -= reelectingConsulPenalty;
            }
            politicians[i].changeOffice();
        }
        politicians[i].yearsOfService++; //after the election, increase the years of service of the politician
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


void annual_influx(std::vector<Politician>& politicians) {
    std::random_device rd;
    std::default_random_engine gen(rd()); 
    std::normal_distribution<> dist(15, 5); // set the normal distribution with mean 15 and std 5
    
    int num_new_candidates = std::round(dist(gen)); // set the number of new candidates
    for (int i = 0; i < num_new_candidates; ++i) {
        int initial_age = 30 + std::round(dist(gen)); // the initial age of the candidate is between 30 and 42
        int death_age;
        do {
            death_age = 30 + std::round(dist(gen)); 
        } while (death_age < initial_age); // check that death age is not smaller than initial age

        Politician new_candidate(initial_age, death_age);
        politicians.push_back(new_candidate); // finally add the new candidate to the list
    }
}

int main() {
    // initialize the politicians. totalNumOfPoliticians is 40
    std::vector<Politician> politicians = initialize_politicians(totalNumOfPoliticians);
    int psi = startingPSI; //startingPSI is 100

    // execute the simulation 200 times
    for (int year = 0; year < simulationYear; ++year) {
       
        electionSimulation(politicians, psi);
        update_politician_ages(politicians);
        annual_influx(politicians);

        // if the psi is smaller than zero, then stop the simulation. If not, then continue
        if (psi <= 0) {
            std::cout << "End-of-Simulation PSI: 0" << std::endl;
            break;
        }
    }

    
    std::cout << "End-of-Simulation PSI: " << psi << std::endl;
    

    // annual fill rates of the positions and print them as percentage
    double fillRateQua = static_cast<double>(count_politicians(politicians, "Quaestor")) / totalNumOfPoliticians * 100.0;
    double fillRateAed = static_cast<double>(count_politicians(politicians, "Aedile")) / totalNumOfPoliticians * 100.0;
    double fillRatePra = static_cast<double>(count_politicians(politicians, "Praetor")) / totalNumOfPoliticians * 100.0;
    double fillRateCon = static_cast<double>(count_politicians(politicians, "Consul")) / totalNumOfPoliticians * 100.0;
    std::cout << "Annual Fill Rate:" << std::endl;
    std::cout << "Quaestor: " << fillRateQua << "%" << std::endl;
    std::cout << "Aedile: " << fillRateAed << "%"  << std::endl;
    std::cout << "Praetor: " << fillRatePra << "%" << std::endl;
    std::cout << "Consul: " << fillRateCon << "%" << std::endl;

    // find the maximum age of the politicians
    int maxAge = 0;
    for (size_t i = 0; i < politicians.size(); ++i) {
        if (politicians[i].age > maxAge) {
            maxAge = politicians[i].age;
        }
    }

    // create vectors to hold how many politicians are ther in each age until the maximum age
    std::vector<int> ageCountQua(maxAge + 1, 0);
    std::vector<int> ageCountAed(maxAge + 1, 0);
    std::vector<int> ageCountPra(maxAge + 1, 0);
    std::vector<int> ageCountCon(maxAge + 1, 0);

    // count how many politicians are ther in each age
    for (size_t i = 0; i < politicians.size(); ++i) {
        if (politicians[i].position == "Quaestor") {
            ageCountQua[politicians[i].age]++;
        } else if (politicians[i].position == "Aedile") {
            ageCountAed[politicians[i].age]++;
        } else if (politicians[i].position == "Praetor") {
            ageCountPra[politicians[i].age]++;
        } else if (politicians[i].position == "Consul") {
            ageCountCon[politicians[i].age]++;
        }
    }

    // print the age distribution of each office
    std::cout << "Age Distribution:\n";
    std::cout << "Quaestor:\n";
    for (int age = min_age_qua; age <= maxAge; ++age) {
        if (ageCountQua[age] > 0) {
            std::cout << "Age " << age << ": " << ageCountQua[age] << " politicians\n";
        }
    }

    std::cout << "Aedile:\n";
    for (int age = min_age_aed; age <= maxAge; ++age) {
        if (ageCountAed[age] > 0) {
            std::cout << "Age " << age << ": " << ageCountAed[age] << " politicians\n";
        }
    }

    std::cout << "Praetor:\n";
    for (int age = min_age_pra; age <= maxAge; ++age) {
        if (ageCountPra[age] > 0) {
            std::cout << "Age " << age << ": " << ageCountPra[age] << " politicians\n";
        }
    }

    std::cout << "Consul:\n";
    for (int age = min_age_con; age <= maxAge; ++age) {
        if (ageCountCon[age] > 0) {
            std::cout << "Age " << age << ": " << ageCountCon[age] << " politicians\n";
        }
    }
    return 0;
}
