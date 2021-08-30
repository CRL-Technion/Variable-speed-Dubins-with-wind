#include<iostream> // std::cout, std::endl
#include<cmath>
#include<math.h>

#include<RobustDubins_Solver.h>
#include<MathTools.h>

// constructor
RobustDubins::Solver::Solver() {
    m_distanceErrorTolerance = 0.001;
    m_thetaErrorTolerance = 0.001;
    m_costTolerance = 0.0001;
    m_psNormalizedFlag = false;
    m_optimalSolutionID = -1;
}

// set functions 

void RobustDubins::Solver::set_problemStatement(RobustDubins::Problem& problemStatement) {
    // check if input problem is defined 
#ifndef NDEBUG
    if (!problemStatement.isDefined()) {
        throw std::runtime_error("RobustDubins::Solver::set_problemStatement: invalid entry");
    }
#endif
    m_problemStatementInput = problemStatement;
    // check if input problem is normalized
    if ((problemStatement.get_startPtInputFlag() == true) ||
        (problemStatement.get_minTurningRadiusInputFlag() == true)) {
        normalizeProblem();
    }
    else { // problem is already normalized, nothing to do
        m_problemStatementNorm = problemStatement;
    }
    // for convenience define these
    m_cth = cos(m_problemStatementNorm.get_hFinal());
    m_sth = sin(m_problemStatementNorm.get_hFinal());
}

void RobustDubins::Solver::set_distanceErrorTolerance(const double& distanceErrorTolerance) {
#ifndef NDEBUG
    if (m_distanceErrorTolerance <= 0) {
        throw std::runtime_error("RobustDubins::Solver::set_distanceErrorTolerance: invalid value.");
    }
#endif
    m_distanceErrorTolerance = distanceErrorTolerance;
}

void RobustDubins::Solver::set_thetaErrorTolerance(const double& thetaErrorTolerance) {
#ifndef NDEBUG
    if (m_thetaErrorTolerance <= 0) {
        throw std::runtime_error("RobustDubins::Solver::set_thetaErrorTolerance: invalid value.");
    }
#endif
    m_thetaErrorTolerance = thetaErrorTolerance;
}

void RobustDubins::Solver::set_costTolerance(const double& costTolerance) {
#ifndef NDEBUG
    if (m_costTolerance <= 0) {
        throw std::runtime_error("RobustDubins::Solver::set_costTolerance: invalid value.");
    }
#endif
    m_costTolerance = costTolerance;
}


// main functions 

void RobustDubins::Solver::normalizeProblem() {
    // translate
    double xFinalTranslated = m_problemStatementInput.get_xFinal() -
        m_problemStatementInput.get_xInitial();
    double yFinalTranslated = m_problemStatementInput.get_yFinal() -
        m_problemStatementInput.get_yInitial();
    // scale
    double R = m_problemStatementInput.get_minTurnRadius();
    double xFinalScaled = xFinalTranslated / R;
    double yFinalScaled = yFinalTranslated / R;
    double hInitial = m_problemStatementInput.get_hInitial();
    // rotate
    double xFinalRotated = xFinalScaled * cos(-hInitial)
        - yFinalScaled * sin(-hInitial);
    double yFinalRotated = xFinalScaled * sin(-hInitial)
        + yFinalScaled * cos(-hInitial);
    // adjust final heading
    double hFinalRotated = MathTools::mod(m_problemStatementInput.get_hFinal()
        - hInitial, 2.0 * M_PI);
    // define the normalized problem
    m_problemStatementNorm.set_stateInitial(0.0, 0.0, 0.0);
    m_problemStatementNorm.set_stateFinal(xFinalRotated, yFinalRotated,
        hFinalRotated);

    // TODO - normalize wind
    m_problemStatementNorm.wind_x = m_problemStatementInput.wind_x;
    m_problemStatementNorm.wind_y = m_problemStatementInput.wind_y;
    // flag indicating problem was normalized 
    m_psNormalizedFlag = true;
}

void RobustDubins::Solver::solve() {
    m_LSL.set_pathType("LSL");
    m_LSR.set_pathType("LSR");
    m_RSL.set_pathType("RSL");
    m_RSR.set_pathType("RSR");
    m_LRL.set_pathType("LRL");
    m_RLR.set_pathType("RLR");
    // we operate on the normalized problem first 
    // check if initial state is eq. to final state (in this case, no solving)
    if (compareEndpoints(m_problemStatementNorm.get_stateInitial(),
        m_problemStatementNorm.get_stateFinal())) {
        std::cout << "RobustDubins::Solver: start/endpoint are equal!" << std::endl;
        m_optimalSolutionType = "LSL"; // default choice 
        m_optimalSolutionID = 0;
        m_LSL.set_solutionStatus("optimal");
        m_LSL.set_cost();
        m_optCost = 0;
    }
    else {
        solveBSB();
        solveBBB();
        if (m_psNormalizedFlag) {
            transformSolutions();
        }
        compareCandidates(); // compare results to find optimal
    }
}

void RobustDubins::Solver::solveBBB() {
    // terminal conditions
    double x = m_problemStatementNorm.get_xFinal(); // final planar positions
    double y = m_problemStatementNorm.get_yFinal();
    double h = MathTools::mod(m_problemStatementNorm.get_hFinal(), 2.0 * M_PI);
    // initialize variables
    vd bCandidate(2);
    vd aCandidate(4);
    double A, B, arg_atan_y, arg_atan_x, a, aSmall, aBig, c;
    bool endPtSat, condBBBSat;
    bool solutionFound = false;
    double v, w;

    for (int k = 4; k < 6; k++) {
        m_solnPtrs[k]->set_solutionStatus("infeasible");
        m_solnPtrs[k]->set_cost();
    }

/*
    for (int k = 4; k < 6; k++) {
        std::string pathType = m_solnPtrs[k]->get_pathType();
        if (pathType.compare("RLR") == 0) {
            v = (x + sin(h)) / 2.0; /// see Tang p.3
            w = (-y - 1.0 + cos(h)) / 2.0;
        }
        else if (pathType.compare("LRL") == 0) {
            v = (x - sin(h)) / 2.0; /// see Tang p.3
            w = (y - 1.0 + cos(h)) / 2.0;
        }
        bCandidate.at(0) = (acos(1 - (v * v + w * w) / 2.0)); // returns a soln [-pi/2,pi/2];
    // check if bCandidate is "nan". if so, we do not proceed
        if (std::isnan(bCandidate.at(0))) {
            m_solnPtrs[k]->set_solutionStatus("infeasible");
            m_solnPtrs[k]->set_cost();
        }
        else {
            // otherwise continue
            if (bCandidate.at(0) < 0) { // make positive
                bCandidate.at(0) = -bCandidate.at(0);
            }
            bCandidate.at(1) = (2.0 * M_PI - bCandidate[0]);
            for (int i = 0; i < 2; i++) { // for each b candidate
                A = (v * v - w * w) / (2.0 * (1.0 - cos(bCandidate[i])));
                B = v * w / (1.0 - cos(bCandidate[i]));
                arg_atan_y = (B * cos(bCandidate[i]) + A * sin(bCandidate[i]));
                arg_atan_x = (A * cos(bCandidate[i]) - B * sin(bCandidate[i]));
                a = 1.0 / 2.0 * atan2(arg_atan_y, arg_atan_x); // returns val from [-pi,pi]
                // check if aCandidate is "nan". if so, we do not proceed
                if (std::isnan(a)) {
                    m_solnPtrs[k]->set_solutionStatus("infeasible");
                    m_solnPtrs[k]->set_cost();
                }
                else {

                    while (a < 0) {
                        a = a + M_PI / 2;
                    }
                    // four possible values on the interval [0, 2*pi]
                    aCandidate.at(0) = MathTools::mod(a, 2.0 * M_PI);
                    aCandidate.at(1) = MathTools::mod(a + M_PI / 2.0, 2.0 * M_PI);
                    aCandidate.at(2) = MathTools::mod(a + M_PI, 2.0 * M_PI);
                    aCandidate.at(3) = MathTools::mod(a + 3.0 * M_PI / 2.0, 2.0 * M_PI);
                    for (int j = 0; j < 4; j++) { // for each a candidate
                        if (pathType.compare("RLR") == 0) {
                            c = MathTools::mod(-h - aCandidate[j] + bCandidate[i], 2.0 * M_PI);
                        }
                        else if (pathType.compare("LRL") == 0) {
                            c = MathTools::mod(h - aCandidate[j] + bCandidate[i], 2.0 * M_PI);
                        }
                        //check if this triple (a,b,c) satisfies endpoint
                        endPtSat = RobustDubins::Solver::checkCandidateEndpoint(pathType,
                            std::abs(aCandidate[j]),
                            std::abs(bCandidate[i]),
                            std::abs(c));
                        // also check if the triple satisfies additional BBB conditions 
                        condBBBSat = RobustDubins::Solver::checkBBBconditions(aCandidate[j],
                            bCandidate[i],
                            c);
                        // if both satisfied, then declare this as a candidate Dubins path
                        if (endPtSat && condBBBSat) {
                            solutionFound = true;
                            m_solnPtrs[k]->set_aParamUnsigned(std::abs(aCandidate[j]));
                            m_solnPtrs[k]->set_bParamUnsigned(std::abs(bCandidate[i]));
                            m_solnPtrs[k]->set_cParamUnsigned(std::abs(c));
                            m_solnPtrs[k]->set_solutionStatus("feasible");
                            m_solnPtrs[k]->set_wind(m_problemStatementNorm.wind_x, m_problemStatementNorm.wind_y);
                            m_solnPtrs[k]->computeEndpoint();
                            m_solnPtrs[k]->set_cost();
                        }
                        if (!solutionFound) {
                            m_solnPtrs[k]->set_solutionStatus("infeasible");
                            m_solnPtrs[k]->set_cost();
                        }
                    }
                }
            }
        }
    }*/
}

double mod2pi(double x){
    while(x >= 2*M_PI) x -= 2*M_PI;
    while(x < 0) x += 2*M_PI;
    return x;
}

double getGroundSpeed(double dir_x, double dir_y, double wind_x, double wind_y, double speed){
    // normalize dir vector
    double tmp_len = sqrt(dir_x * dir_x + dir_y * dir_y);
    dir_x /= tmp_len;
    dir_y /= tmp_len;

    double b = wind_x * dir_x + wind_y * dir_y;
    double c = wind_x * wind_x + wind_y * wind_y - speed * speed;
    return (b + sqrt(b*b - c));
}

double getHeadingFromDirection(double dir_x, double dir_y, double wind_x, double wind_y, double speed){
    double groudSpeed = getGroundSpeed(dir_x, dir_y, wind_x, wind_y, speed);

    // normalize dir vector
    double tmp_len = sqrt(dir_x * dir_x + dir_y * dir_y);
    dir_x /= tmp_len;
    dir_y /= tmp_len;

    double air_x = dir_x * groudSpeed - wind_x;
    double air_y = dir_y * groudSpeed - wind_y;

    //air = normalize(dir) .* groudSpeed .- wind
    return atan2(air_y, air_x);
}

void RobustDubins::Solver::solveBSB() {
    // terminal conditions
    double x = m_problemStatementNorm.get_xFinal(); // final planar positions
    double y = m_problemStatementNorm.get_yFinal();
    double h = MathTools::mod(m_problemStatementNorm.get_hFinal(), 2.0 * M_PI);

    double xi = m_problemStatementInput.get_xInitial();
    double yi = m_problemStatementInput.get_yInitial();
    double hi = m_problemStatementInput.get_hInitial();

    double xf = m_problemStatementInput.get_xFinal();
    double yf = m_problemStatementInput.get_yFinal();
    double hf = m_problemStatementInput.get_hFinal();

    double wind_x = m_problemStatementInput.wind_x;
    double wind_y = m_problemStatementInput.wind_y;

    double si = sin(hi);
    double ci = cos(hi);
    double sf = sin(hf);
    double cf = cos(hf);

    double a, b, c;
    double arg_atan_y, arg_atan_x;
    for (int k = 0; k < 4; k++) { // solnPtrs[2-6] : LSL, LSR, RSL, RSR
        std::string pathType = m_solnPtrs[k]->get_pathType();
        if (pathType.compare("LSL") == 0) {
            for(int loop = 0; loop <= 2; loop++){
                double turn = mod2pi(hf - hi) + loop * 2 * M_PI;
                // difference of turn origins
                double o_dx =  si -sf +xf -xi -turn*wind_x;
                double o_dy = -ci +cf +yf -yi -turn*wind_y;
                double dir = getHeadingFromDirection(o_dx, o_dy, wind_x, wind_y, 1.);
                // compute lengths of the segments
                a = mod2pi(-hi + dir);
                b = sqrt(o_dx*o_dx + o_dy*o_dy) * 1 / getGroundSpeed(o_dx, o_dy, wind_x, wind_y, 1.); //TODO flag to find it
                c = turn - a;
                if(c > 0.){
                    m_solnPtrs[k]->set_aParamUnsigned(a);
                    m_solnPtrs[k]->set_bParamUnsigned(b);
                    m_solnPtrs[k]->set_cParamUnsigned(c);
                    m_solnPtrs[k]->set_solutionStatus("feasible");
                    m_solnPtrs[k]->set_wind(wind_x, wind_y);
                    m_solnPtrs[k]->computeEndpoint();
                    m_solnPtrs[k]->set_cost();  
                    m_solnPtrs[k]->print();
                    break;
                }
            }
        } else if (pathType.compare("RSR") == 0) {
            for(int loop = 0; loop <= 2; loop++){
                double turn = -mod2pi(-hf + hi) - loop * 2 * M_PI;
                // difference of turn origins
                double o_dx = -si +sf +xf -xi +turn*wind_x;
                double o_dy =  ci -cf +yf -yi +turn*wind_y;
                double dir = getHeadingFromDirection(o_dx, o_dy, wind_x, wind_y, 1.);
                // compute lengths of the segments
                a = -mod2pi(hi - dir);
                b = sqrt(o_dx*o_dx + o_dy*o_dy) * 1 / getGroundSpeed(o_dx, o_dy, wind_x, wind_y, 1.);
                c = turn - a;
                if(c < 0.){
                    m_solnPtrs[k]->set_aParamUnsigned(-a);
                    m_solnPtrs[k]->set_bParamUnsigned(b);
                    m_solnPtrs[k]->set_cParamUnsigned(-c);
                    m_solnPtrs[k]->set_solutionStatus("feasible");
                    m_solnPtrs[k]->set_wind(wind_x, wind_y);
                    m_solnPtrs[k]->computeEndpoint();
                    m_solnPtrs[k]->set_cost();  
                    m_solnPtrs[k]->print();
                    break;
                }
            }
        } else if (pathType.compare("LSR") == 0) {
            // TODO - here
            m_solnPtrs[k]->set_solutionStatus("infeasible");
            m_solnPtrs[k]->set_cost();
        } else if (pathType.compare("RSL") == 0) {
            // TODO - here
            m_solnPtrs[k]->set_solutionStatus("infeasible");
            m_solnPtrs[k]->set_cost();
        } else {
            m_solnPtrs[k]->set_solutionStatus("infeasible");
            m_solnPtrs[k]->set_cost();
        }
    } // end of candidate loop
}

bool RobustDubins::Solver::checkCandidateEndpoint(const std::string& pathType,
    const double& aCandidate,
    const double& bCandidate,
    const double& cCandidate) {
    // generate test endpoint using radius R = 1
    vd testEndpoint(3);
    computeDubinsEndpoint(pathType, std::abs(aCandidate), std::abs(bCandidate),
        std::abs(cCandidate), 0.0, 0.0, 0.0,
        1.0, testEndpoint);
    // return true if testEndpoint = stateFinal
    return compareEndpoints(testEndpoint, m_problemStatementNorm.get_stateFinal());
}

bool RobustDubins::Solver::compareEndpoints(const vd& testState,
    const vd& trueState) {
    // compute distance error
    vd trueEndpoint = { trueState[0], trueState[1] };
    vd testEndpoint = { testState[0], testState[1] };
    double distance_error = MathTools::distance(trueEndpoint, testEndpoint);
    // compute heading error
    double theta_error = MathTools::polarDistance(trueState[2], testState[2]);
    if ((distance_error <= m_distanceErrorTolerance) &&
        (theta_error <= m_thetaErrorTolerance))
        return true;
    else
        return false;
}

bool RobustDubins::Solver::checkBBBconditions(const double& aCandidate,
    const double& bCandidate,
    const double& cCandidate) {
    // check magnitude conditions
    bool condition1 = std::max(aCandidate, cCandidate) < bCandidate;
    bool condition2 = std::min(aCandidate, cCandidate) < bCandidate + M_PI;
    return (condition1 && condition2);
}

void RobustDubins::Solver::transformSolutions() {
    double R = m_problemStatementInput.get_minTurnRadius();
    for (int k = 0; k < 6; k++) {
        m_solnPtrs[k]->set_initialState(m_problemStatementInput.get_xInitial(),
            m_problemStatementInput.get_yInitial(),
            m_problemStatementInput.get_hInitial());
        // if feasible or optimal (i.e., *not* infeasible)
        if (!(m_solnPtrs[k]->get_solutionStatus()).compare("infeasible") == 0) {
            m_solnPtrs[k]->set_minTurnRadius(R);
            double a = m_solnPtrs[k]->get_aParamUnsigned() * R;
            double b = m_solnPtrs[k]->get_bParamUnsigned() * R;
            double c = m_solnPtrs[k]->get_cParamUnsigned() * R;
            m_solnPtrs[k]->set_abcParamVectorUnsigned(a, b, c);
            // endpoint should be equal (approx.) to problem statement final point 
            // if the path was computed correctly  
            m_solnPtrs[k]->computeEndpoint();
        }
    }
}

void RobustDubins::Solver::compareCandidates() {
    // store costs
    m_costVector.push_back(m_LSL.get_cost());
    m_costVector.push_back(m_LSR.get_cost());
    m_costVector.push_back(m_RSL.get_cost());
    m_costVector.push_back(m_RSR.get_cost());
    m_costVector.push_back(m_LRL.get_cost());
    m_costVector.push_back(m_RLR.get_cost());
    // compare costs
    m_minCostPaths = MathTools::minIndicesWithTolerance(m_costVector,
        m_costTolerance);
    m_numMinCostPaths = m_minCostPaths.size();
    determineSolutionType();
}

// determineSolutionType
void RobustDubins::Solver::determineSolutionType() {
    // assume that: 0-LSL, 1-LSR, 2-RSL, 3-RSR, 4-LRL, 5-RLR, 6-LSR/RSR, 7-LRL/RLR
    if (m_numMinCostPaths == 1) {
        if (m_minCostPaths[0] == 0) { // LSL (only)
            m_optimalSolutionType = "LSL";
            m_optimalSolutionID = 0;
            m_LSL.set_solutionStatus("optimal");
            m_optCost = m_LSL.get_cost();
        }
        else if (m_minCostPaths[0] == 1) { // LSR (only)
            m_optimalSolutionType = "LSR";
            m_optimalSolutionID = 1;
            m_LSR.set_solutionStatus("optimal");
            m_optCost = m_LSR.get_cost();
        }
        else if (m_minCostPaths[0] == 2) { // RSL (only)
            m_optimalSolutionType = "RSL";
            m_optimalSolutionID = 2;
            m_RSL.set_solutionStatus("optimal");
            m_optCost = m_RSL.get_cost();
        }
        else if (m_minCostPaths[0] == 3) { // RSR (only)
            m_optimalSolutionType = "RSR";
            m_optimalSolutionID = 3;
            m_RSR.set_solutionStatus("optimal");
            m_optCost = m_RSR.get_cost();
        }
        else if (m_minCostPaths[0] == 4) { // LRL (only)
            m_optimalSolutionType = "LRL";
            m_optimalSolutionID = 4;
            m_LRL.set_solutionStatus("optimal");
            m_optCost = m_LRL.get_cost();
        }
        else if (m_minCostPaths[0] == 5) { // RLR (only)
            m_optimalSolutionType = "RLR";
            m_optimalSolutionID = 5;
            m_RLR.set_solutionStatus("optimal");
            m_optCost = m_RLR.get_cost();
        }
    }
    // if two paths have same cost they are either "LSL/RSR" or "LRL/RLR"
    else if (m_numMinCostPaths >= 2) {
        // Case: Two Optimal Solutions
        if ((m_minCostPaths[0] == 0 || m_minCostPaths[0] == 3) &&
            (m_minCostPaths[1] == 0 || m_minCostPaths[1] == 3)) { // LSL-RSR
            m_optimalSolutionType = "LSL-RSR";
            m_optimalSolutionID = 6;
            m_LSL.set_solutionStatus("optimal");
            m_RSR.set_solutionStatus("optimal");
            m_optCost = m_LSL.get_cost();
        }
        else if ((m_minCostPaths[0] == 4 || m_minCostPaths[0] == 5) &&
            (m_minCostPaths[1] == 4 || m_minCostPaths[1] == 5)) {
            // LRL-RLR
            m_optimalSolutionType = "LRL-RLR";
            m_optimalSolutionID = 7;
            m_LRL.set_solutionStatus("optimal");
            m_RLR.set_solutionStatus("optimal");
            m_optCost = m_LRL.get_cost();
        }
        // Case: Degenerate, some parameters are zero, we default to the first
        // min cost path
        else {
            m_optimalSolutionType = "degenerate";
            m_optimalSolutionID = m_minCostPaths[0];
            if (m_optimalSolutionID == 0) {
                m_LSL.set_solutionStatus("optimal");
                m_optCost = m_LSL.get_cost();
            }
            else if (m_optimalSolutionID == 1) {
                m_LSR.set_solutionStatus("optimal");
                m_optCost = m_LSR.get_cost();
            }
            else if (m_optimalSolutionID == 2) {
                m_RSL.set_solutionStatus("optimal");
                m_optCost = m_RSL.get_cost();
            }
            else if (m_optimalSolutionID == 3) {
                m_RSR.set_solutionStatus("optimal");
                m_optCost = m_RSR.get_cost();
            }
            else if (m_optimalSolutionID == 4) {
                m_LRL.set_solutionStatus("optimal");
                m_optCost = m_LRL.get_cost();
            }
            else if (m_optimalSolutionID == 5) {
                m_RLR.set_solutionStatus("optimal");
                m_optCost = m_RLR.get_cost();
            }
        }
    }
}

void RobustDubins::Solver::print() {
    printf("-------------------------------------------------\n");
    printf("          RobustDubins::Solver Output          \n");
    printf("-------------------------------------------------\n");
    printf("Type, Status, Cost, a, b, c\n");
    for (int i = 0; i < 6; i++) {
        // get solution status and path type
        if ((m_solnPtrs[i]->get_solutionStatus()).compare("infeasible") == 0) {
            printf("%i) %s , %s , inf , %3.3f, %3.3f, %3.3f \n", i,
                (m_solnPtrs[i]->get_solutionStatus()).c_str(),
                (m_solnPtrs[i]->get_pathType()).c_str(),
                m_solnPtrs[i]->get_aParamUnsigned(),
                m_solnPtrs[i]->get_bParamUnsigned(),
                m_solnPtrs[i]->get_cParamUnsigned());
        }
        else {
            printf("%i) %s , %s , %3.3f , %3.3f, %3.3f, %3.3f \n", i,
                (m_solnPtrs[i]->get_solutionStatus()).c_str(),
                (m_solnPtrs[i]->get_pathType()).c_str(),
                m_solnPtrs[i]->get_cost(),
                m_solnPtrs[i]->get_aParamUnsigned(),
                m_solnPtrs[i]->get_bParamUnsigned(),
                m_solnPtrs[i]->get_cParamUnsigned());
        }
    }
}

void RobustDubins::Solver::writeOctaveCommandsToPlotSolution(
    const std::string& fileName,
    const int& figNum) {
    // iterate through the elements of m_solnPtrs
    for (int i = 0; i < 6; i++) {
        // get solution status and path type
        std::string solnStatus = m_solnPtrs[i]->get_solutionStatus();
        std::string pathType = m_solnPtrs[i]->get_pathType();
        // if solution found, compute path
        if (solnStatus.compare("feasible") == 0 ||
            solnStatus.compare("optimal") == 0) {
            m_solnPtrs[i]->computePathHistory();
            // if optimal, plot as solid red line 
            if (solnStatus.compare("optimal") == 0) {
                m_solnPtrs[i]->writePathOctavePlotCommands(fileName, figNum,
                    "x_" + pathType + "_opt", "y_" + pathType + "_opt", "r");
            }
            // if suboptimal, plot as dashed black line 
            else {
                m_solnPtrs[i]->writePathOctavePlotCommands(fileName, figNum,
                    "x_" + pathType, "y_" + pathType, "k--");
            }
        }
    }

}

// get functions
RobustDubins::Path RobustDubins::Solver::get_optimalPath() {
    // assume that: 
  // 0-LSL, 
  // 1-LSR,
  // 2-RSL, 
  // 3-RSR, 
  // 4-LRL, 
  // 5-RLR, 
  // 6-LSL/RSR (both optimal, but we default to LSL)
    // 7-LRL/RLR (both optimal, but we default to LRL)
    if (m_optimalSolutionID == 0) {
        return m_LSL;
    }
    else if (m_optimalSolutionID == 1) {
        return m_LSR;
    }
    else if (m_optimalSolutionID == 2) {
        return m_RSL;
    }
    else if (m_optimalSolutionID == 3) {
        return m_RSR;
    }
    else if (m_optimalSolutionID == 4) {
        return m_LRL;
    }
    else if (m_optimalSolutionID == 5) {
        return m_RLR;
    }
    // when two solutions exist the one beginning with a left turn is returned
    else if (m_optimalSolutionID == 6) {
        return m_LSL;
    }
    else if (m_optimalSolutionID == 7) {
        return m_LRL;
    }
}

void RobustDubins::Solver::get_optimalWaypoints(vd& x, vd& y, vd& h) {
    int optSolnID;
    // if there are two solutions, return the left handed one
    if (m_optimalSolutionID == 6) {
        optSolnID = 0; // LSL
    }
    else if (m_optimalSolutionID == 7) {
        optSolnID = 4; // LRL 
    }
    else {
        optSolnID = m_optimalSolutionID;
    }
    m_solnPtrs[optSolnID]->computePathHistory();
    x = m_solnPtrs[optSolnID]->get_xHistory();
    y = m_solnPtrs[optSolnID]->get_yHistory();
    h = m_solnPtrs[optSolnID]->get_hHistory();
}

void RobustDubins::Solver::get_optimalWaypointsSetSpacing(vd& x, vd& y, vd& h, double spacing) {
    int optSolnID;
    // if there are two solutions, return the left handed one
    if (m_optimalSolutionID == 6) {
        optSolnID = 0; // LSL
    }
    else if (m_optimalSolutionID == 7) {
        optSolnID = 4; // LRL 
    }
    else {
        optSolnID = m_optimalSolutionID;
    }
    m_solnPtrs[optSolnID]->set_spacing(spacing);
    m_solnPtrs[optSolnID]->computePathHistory();
    x = m_solnPtrs[optSolnID]->get_xHistory();
    y = m_solnPtrs[optSolnID]->get_yHistory();
    h = m_solnPtrs[optSolnID]->get_hHistory();
}
