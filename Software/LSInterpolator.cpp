#include "LSInterpolator.h"

#include <string>

using namespace net;

/// PUBLIC METHODS

LSInterpolator::LSInterpolator() {
	smoothingFactor = 0;
	e = 0.01;
}

LSInterpolator::LSInterpolator(double smoothingFactor_, double e_) {
	smoothingFactor = smoothingFactor_;
	e = e_;
}

bool LSInterpolator::initFromFile(std::ifstream *in) {
	std::string name;
	if (!(*in >> name)) {
		std::cout << "Interpolator could not read ifstrem\n";
		throw 1;
	}

	if (name != "LS") {
		return false;
	}

	*in >> smoothingFactor >> e;
	return true;
}

void LSInterpolator::storeInterpolator(std::ofstream *out) {
	*out << " LS " << smoothingFactor << " " << e << " ";
}

double LSInterpolator::getReward(const std::vector<Wire> &controlWires, const std::vector<double> &action) {
	double maxReward = -9999999;
	for (auto a = controlWires.begin(); a != controlWires.end(); ++a) if (a->reward > maxReward) maxReward = a->reward;

    return weightedSum(controlWires, action, maxReward) / normalize(controlWires, action, maxReward);
}

double LSInterpolator::rewardDerivative(const std::vector<double> &action, const Wire &wire, const std::vector<Wire> &controlWires) {
	double maxReward = -9999999;
    for(auto a = controlWires.begin(); a != controlWires.end(); ++a) if(a->reward > maxReward) maxReward = a->reward;
    
    double norm = normalize(controlWires, action, maxReward), wsum = weightedSum(controlWires, action, maxReward), distance = distanceBetweenWireAndAction(wire, action, maxReward);
    
    return (norm * (distance + wire.reward*smoothingFactor) - wsum*smoothingFactor) / pow(norm * distance, 2);
}

double LSInterpolator::actionTermDerivative(double actionTerm, double wireActionTerm, const std::vector<double> &action, const Wire &wire, const std::vector<Wire> &controlWires) {
	double maxReward = -9999999;
    for(auto a = controlWires.begin(); a != controlWires.end(); ++a) if(a->reward > maxReward) maxReward = a->reward;
    
    double norm = normalize(controlWires, action, maxReward), wsum = weightedSum(controlWires, action, maxReward), distance = distanceBetweenWireAndAction(wire, action, maxReward);
    
    return ((wsum - norm*wire.reward) * 2 * (wireActionTerm - actionTerm)) / pow(norm*distance, 2);
}

/// PRIVATE METHODS

double LSInterpolator::distanceBetweenWireAndAction(const Wire &wire, const std::vector<double> &action, double maxReward) {
	double euclideanNorm = 0;
    for(int a = 0; a < action.size(); a++) euclideanNorm += pow(action[a] - wire.action[a], 2);
    euclideanNorm = sqrt(euclideanNorm);

    return pow(euclideanNorm, 2) + smoothingFactor*(maxReward - wire.reward) + e;
}

double LSInterpolator::weightedSum(const std::vector<Wire> &wires, const std::vector<double> &action, double maxReward) {
	double answer = 0;
    for(auto a = wires.begin(); a != wires.end(); ++a) {
        answer += a->reward / distanceBetweenWireAndAction(*a, action, maxReward);
    }
    
    return answer;
}

double LSInterpolator::normalize(const std::vector<Wire> &wires, const std::vector<double> &action, double maxReward) {
	double answer = 0;
    for(auto a = wires.begin(); a != wires.end(); ++a) {
        answer += 1.0 / distanceBetweenWireAndAction(*a, action, maxReward);
    }
    
    return answer;
}