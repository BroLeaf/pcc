#include <ccc.h>
#include <cmath>
#include <iostream>
#include <udt.h>

#define MAXCOUNT 1000
#define GRANULARITY 0.01
#define MUTATION_TH 300
#define JUMP_RANGE 0.05
#define NUMBER_OF_PROBE 4
#define MAX_COUNTINOUS_GUESS 5
#define MAX_COUNTINOUS_SEND 1
#define MAX_MONITOR_NUMBER 100

using namespace std;
class CTCP : public CCC {

     int dir;

 public:
     void init()
     {
          m_bSlowStart = true;
          m_issthresh = 83333;

          m_dPktSndPeriod = 10000.0;
          m_dCWndSize = 2.0;

          setACKInterval(1);
          setRTO(2000000);
     }

     virtual void onACK(const int& ack)
     {
          if (ack == m_iLastACK) {
               if (3 == ++m_iDupACKCount)
                    DupACKAction();
               else if (m_iDupACKCount > 3)
                    m_dCWndSize += 1.0;
               else
                    ACKAction();
          } else {
               if (m_iDupACKCount >= 3)
                    m_dCWndSize = m_issthresh;

               m_iLastACK = ack;
               m_iDupACKCount = 1;

               ACKAction();
          }
     }

     virtual void onTimeout()
     {
          m_issthresh = getPerfInfo()->pktFlightSize / 2;
          if (m_issthresh < 2)
               m_issthresh = 2;

          m_bSlowStart = true;
          m_dCWndSize = 2.0;
     }

 protected:
     virtual void ACKAction()
     {
          if (m_bSlowStart) {
               m_dCWndSize += 1.0;

               if (m_dCWndSize >= m_issthresh)
                    m_bSlowStart = false;
          } else
               m_dCWndSize += 1.0 / m_dCWndSize;
     }

     virtual void DupACKAction()
     {
          m_bSlowStart = false;

          m_issthresh = getPerfInfo()->pktFlightSize / 2;
          if (m_issthresh < 2)
               m_issthresh = 2;

          m_dCWndSize = m_issthresh + 3;
     }

 protected:
     int m_issthresh;
     bool m_bSlowStart;

     int m_iDupACKCount;
     int m_iLastACK;
};

class BBCC : public CCC {
 public:
     double utility_array[MAXCOUNT];
     double rate_array[MAXCOUNT];
     int array_pointer;
     int target_monitor;
     int starting_phase;
     int make_guess;
     int guess_result;
     int moving_phase;
     int moving_phase_initial;
     int find_max;
     int random_guess;
     int random_guess_result;
     int mutation_counter;
     double rate_new;
     int change_intense;
     double change_amount;
     int change_direction;
     double rate_bucket[NUMBER_OF_PROBE];
     int monitor_bucket[NUMBER_OF_PROBE];
     double utility_bucket[NUMBER_OF_PROBE];
     int recorded_number;
     double current_rate;
     double previous_rate;
     double current_utility;
     double previous_utility;
     int guess_time;
     int continous_guess_count;
     int continous_send;
     int continous_send_count;
     int recording_guess_result;
     int baseline;
     double utility_baseline;

 public:
     BBCC()

     {
          m_dPktSndPeriod = 70;
          m_dCWndSize = 100000.0;
          setRTO(100000000);
          starting_phase = 1;
          target_monitor = 0;
          make_guess = 0;
          guess_result = 0;
          moving_phase = 0;
          moving_phase_initial = 0;
          change_direction = 0;
          change_intense = 1;
          guess_time = 0;
          continous_guess_count = 0;
          continous_send = 0;
          continous_send_count = 0;
          previous_utility = 0;
          previous_rate = 143;
          current_rate = 143;
          recording_guess_result = 0;
          recorded_number = 0;
          baseline = 0;
          utility_baseline = 0;
     }

 public:
     virtual void onLoss(const int32_t*, const int&)
     {
     }
     virtual void onTimeout()
     {
     }
     /*	int findmax(double arr[]){
		int tmp=0;
		for(int i=0;i<1000;i++){
			if(arr[i]>arr[tmp]&&stale[i]<20)
			{
				tmp=i;

			}
		}
		return tmp;
	}*/

     virtual void onMonitorStart(int current_monitor)
     {

          if (make_guess == 1) {
               //cerr<<"make guess!"<<continous_guess_count<<endl;
               if (guess_time == 0 && continous_guess_count == MAX_COUNTINOUS_GUESS)
                    //cerr<<"skip guess"<<endl;
                    continous_guess_count = 0;
               if (guess_time == 0) {
                    recording_guess_result = 1;
                    continous_guess_count++;

                    int rand_dir;

                    for (int i = 0; i < NUMBER_OF_PROBE; i++) {
                         rand_dir = (rand() % 2 * 2 - 1);
                         rate_bucket[i] = current_rate + rand_dir * continous_guess_count * GRANULARITY * current_rate;
                         rate_bucket[++i] = current_rate - rand_dir * continous_guess_count * GRANULARITY * current_rate;
                         //cerr<<"guess rate"<<rate_bucket[i-1]<<" "<<rate_bucket[i]<<endl;
                    }
                    for (int i = 0; i < NUMBER_OF_PROBE; i++) {
                         monitor_bucket[i] = (current_monitor + i) % MAX_MONITOR_NUMBER;
                         //cerr<<"guess monitor"<<monitor_bucket[i]<<endl;
                    }
               }
               setRate(rate_bucket[guess_time]);
               //cerr<<"setrate as "<<rate_bucket[guess_time]<<endl;
               guess_time++;
               //TODO:Here the sender stopped at a particular rate
               if (guess_time == NUMBER_OF_PROBE) {
                    //cerr<<"Guess exit!"<<endl;
                    make_guess = 0;
                    guess_time = 0;
               }
          }
          if (continous_send == 1) {
               //cerr<<"CONTINOUS send"<<endl;
               if (continous_send_count == 1) {
                    setRate(current_rate);
               }
               if (continous_send_count < MAX_COUNTINOUS_SEND) {
                    continous_send_count++;
                    //cerr<<"continous send"<<endl;

               } else {
                    //cerr<<"clear continous send"<<endl;
                    continous_send = 0;
                    continous_send_count = 0;
                    continous_guess_count = 0;
                    make_guess = 1;
               }
          }
     }

     virtual void onMonitorEnds(int total, int loss, double time, int current, int endMonitor)
     {

          double utility;
          double t = total;
          double l = loss;
          if (l < 0)
               l = 0;
          utility = ((t - l) / time - 20 * l / time);

          //utility = ((t-l)/time*(1-1/(1+exp(-100*(l/t-0.05))))-1*l/time);
          cout << "n " << t * 12 / time / 1000 << '\t' << (t - l) * 12 / time / 1000 << '\t' << t << '\t' << l << '\t' << time << "\t" << utility << "\t" << endMonitor << "\t" << current_rate << endl;
          //		utility = (t-l)/time*(1-l/t)*(1-l/t)*(1-l/t)*(1-l/t)*(1-l/t);(m_iRTT/1000);(m_iRTT/1000);

          if (starting_phase) {
               if (endMonitor >= target_monitor) {
                    target_monitor = (current + 1) % 100;
                    current_utility = utility;

                    if (current_utility >= previous_utility) {
                         previous_rate = current_rate;
                         current_rate = current_rate * 2;
                         setRate(current_rate);
                         previous_utility = current_utility;
                         return;
                    } else if (current_utility < previous_utility) {
                         starting_phase = 0;
                         setRate(previous_rate);
                         //TODO: this part is kind of foggy
                         previous_utility = utility;
                         current_rate = previous_rate;
                         make_guess = 1;
                         return;
                    }
               }
          }

          if (recording_guess_result) {

               for (int i = 0; i < NUMBER_OF_PROBE; i++) {
                    if (endMonitor == monitor_bucket[i]) {
                         recorded_number++;
                         utility_bucket[i] = utility;
                    }
                    //TODO to let the sender go back to the current sending rate, one way is to
                    //let the decision maker stop for another monitor period,which might not be a good option, let's try this first
                    if (recorded_number == NUMBER_OF_PROBE) {
                         recorded_number = 0;
                         double decision = 0;
                         for (int i = 0; i < NUMBER_OF_PROBE; i++) {
                              // TODO: I don't use this because I am afraid of the calculation precision problem
                              // decision += (utility_bucket[i]>utility_bucket[i+1])?(rate_bucket[i]-rate_bucket[i+1]):(rate_bucket[i+1]-rate_bucket[i]);
                              //cerr<<"utility"<<utility_bucket[i]<<" "<<utility_bucket[i+1]<<endl;
                              //cerr<<"rate"<<rate_bucket[i]<<" "<<rate_bucket[i+1]<<endl;
                              if (((utility_bucket[i] > utility_bucket[i + 1]) && (rate_bucket[i] > rate_bucket[i + 1])) || ((utility_bucket[i] < utility_bucket[i + 1]) && (rate_bucket[i] < rate_bucket[i + 1])))
                                   decision += 1;
                              else
                                   decision -= 1;
                              i++;
                         }
                         if (decision == 0) {
                              make_guess = 1;
                              recording_guess_result = 0;
                              //cerr<<"no decision"<<endl;
                         } else {
                              change_direction = decision > 0 ? 1 : -1;
                              //cerr<<"change to the direction of"<<change_direction<<endl;
                              recording_guess_result = 0;
                              target_monitor = (current + 1) % MAX_MONITOR_NUMBER;
                              moving_phase_initial = 1;
                              change_intense = 1;
                              change_amount = (continous_guess_count / 2 + 1) * change_intense * change_direction * GRANULARITY * current_rate;
                              previous_utility = 0;
                              continous_guess_count--;
                              continous_guess_count = 0;
                              if (continous_guess_count < 0)
                                   continous_guess_count = 0;
                              previous_rate = current_rate;
                              current_rate = current_rate + change_amount;
                              target_monitor = (current + 1) % MAX_MONITOR_NUMBER;
                              setRate(current_rate);
                         }
                    }
               }
          }

          if (moving_phase_initial && endMonitor == target_monitor) {
               //cerr<<"moving initial"<<endl;
               target_monitor = (current + 1) % MAX_MONITOR_NUMBER;
               previous_rate = current_rate;
               previous_utility = utility;
               change_intense += 1;
               change_amount = change_intense * GRANULARITY * current_rate * change_direction;
               current_rate = current_rate + change_amount;
               setRate(current_rate);
               moving_phase_initial = 0;
               moving_phase = 1;
          }

          if (moving_phase && endMonitor == target_monitor) {
               //cerr<<"moving quickly"<<endl;
               current_utility = utility;
               if (current_utility > previous_utility) {
                    target_monitor = (current + 1) % MAX_MONITOR_NUMBER;
                    change_intense += 1;
                    previous_utility = current_utility;
                    previous_rate = current_rate;
                    change_amount = change_intense * GRANULARITY * current_rate * change_direction;
                    current_rate = current_rate + change_amount;
                    setRate(current_rate);

               } else {
                    moving_phase = 0;
                    make_guess = 1;
                    //change_intense+=1;
                    previous_utility = current_utility;
                    previous_rate = current_rate;
                    change_amount = change_intense * GRANULARITY * current_rate * change_direction;
                    current_rate = current_rate - change_amount;
                    setRate(current_rate);
               }
          }
     }

     virtual void onACK(const int& ack)
     {
     }

     void setRate(double mbps)
     {
          m_dPktSndPeriod = (m_iMSS * 8.0) / mbps;
     }
};
