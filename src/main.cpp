#include <uWS/uWS.h>
#include <iostream>
#include <string>
#include "json.hpp"
#include "PID.h"
#include <math.h>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main()
{
  uWS::Hub h;

  PID pid;
  // TODO: Initialize the pid variable.
  double p[3] = {0.111222,0.0043165,1.05256};//0.112152,0.00450208,1.05536
  double dp[3] = {0.00093,0.00006, 0.00278};// 0.00093,0.00006, 0.00278
  double tol = 0.01;
  int n=0;
  int n_twiddle=0;
  int max_n=500;
  int ip=0;
  double best_err=10000000000;
  double err=0;
  bool first=true;
  
  h.onMessage([&pid,&n,&max_n,&p,&dp,&ip,&err,&best_err,&first,&tol,&n_twiddle](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          if(n==0){
            pid.Init(p[0],p[1],p[2]); 
          }
          
          double cte = std::stod(j[1]["cte"].get<std::string>());
          err = err + pow(cte,2);
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          double throttle;
          double objective_speed = 50 - 20 * fabs(cte); //60 - 25
          n = n + 1;
          
          if(objective_speed < 10) objective_speed = 10;
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */
          pid.UpdateError(cte);
          steer_value =  pid.Signal();
          if(steer_value > 1.0) steer_value=1.0;
          if(steer_value < -1.0) steer_value=-1.0;
          if(speed < objective_speed){
            throttle = 1;
          }else{
            throttle = 0;
          }
          
          // DEBUG
          //std::cout << n << " CTE: " << cte << " Steering Value: " << steer_value << " Speed: " << speed << " Throttle: " << throttle << std::endl;

          json msgJson;
          std::string msg2;
          
/*          if(n>max_n){
            err = err / max_n;
            if(err < best_err){
              best_err = err;
              if(n_twiddle==0){
                best_err = err;
                p[ip] = p[ip] + dp[ip];
                first= true;
              }else{
                dp[ip] = dp[ip]*1.1;
                ip = (ip+1) % 3;
                p[ip] = p[ip] + dp[ip];
                first= true;
              }
            }else{
              if(first){
                p[ip] = p[ip]-2*dp[ip];
                first = false;                
              }else{
                p[ip] = p[ip] + dp[ip];
                dp[ip] = 0.9 * dp[ip];
                first = true;
                ip = (ip+1) % 3;
                p[ip] = p[ip] + dp[ip];             
              }              
            }
            err = 0;
            n = 0;
            msg2 = "42[\"reset\"]";
            std::cout << "Tol: " << dp[0]+dp[1]+dp[2] << " P: " << p[0] << "," << p[1] << "," << p[2] << " " << " DP: " << dp[0] << "," << dp[1] << "," << dp[2] << std::endl;
            std::cout << "Iteration: " << n_twiddle << " Best error: " << best_err << " Changing parameter: " << ip << std::endl;
            n_twiddle++;
          }else{*/
            msgJson["steering_angle"] = steer_value;
            msgJson["throttle"] = throttle;
            msg2 = "42[\"steer\"," + msgJson.dump() + "]";
          //}
          auto msg = msg2;
          //std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
