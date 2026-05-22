constexpr int right  = A0;
constexpr int middle = A1;
constexpr int left   = A2;

constexpr int trigR = 1; constexpr int echoR = 0; 
constexpr int trigC = 8;  constexpr int echoC = 7;
constexpr int trigL = 4;  constexpr int echoL = 2;

constexpr int m1a = 6;
constexpr int m1b = 5;
constexpr int m2a = 9;
constexpr int m2b = 10;

constexpr float lineKp = 3.8; 
constexpr float lineKd = 0.8; 

constexpr float wallKp = 5.0; 
constexpr float wallKd = 1.0;

constexpr int lineSpeed = 25;
constexpr int wallSpeed = 20; 

void setup() {
  pinMode(left, INPUT);
  pinMode(middle, INPUT);
  pinMode(right, INPUT);

  pinMode(trigL, OUTPUT); pinMode(echoL, INPUT);
  pinMode(trigC, OUTPUT); pinMode(echoC, INPUT);
  pinMode(trigR, OUTPUT); pinMode(echoR, INPUT);

  pinMode(m1a, OUTPUT); pinMode(m1b, OUTPUT);
  pinMode(m2a, OUTPUT); pinMode(m2b, OUTPUT);
  
  Serial.begin(9600);
}

void leftMotor(int hiz) {
  hiz = constexprrain(hiz, -255, 255);
  if (hiz > 0) {
    digitalWrite(m1a, LOW); analogWrite(m1b, hiz);
  } else if (hiz < 0) {
    analogWrite(m1a, -hiz); digitalWrite(m1b, LOW);
  } else {
    digitalWrite(m1a, LOW); digitalWrite(m1b, LOW);
  }
}

void rightMotor(int hiz) {
  hiz = constexprrain(hiz, -255, 255);
  if (hiz > 0) {
    digitalWrite(m2a, LOW); analogWrite(m2b, hiz);
  } else if (hiz < 0) {
    analogWrite(m2a, -hiz); digitalWrite(m2b, LOW);
  } else {
    digitalWrite(m2a, LOW); digitalWrite(m2b, LOW);
  }
}

long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 15000); 
  if (duration == 0) return 50; 
  
  long dist = duration * 0.034 / 2;
  if (dist <= 0 || dist > 50) return 50; 
  
  return dist;
}
int vL, vR, vC, distL, distR, distC;
int line_last_error = 0;
int wall_last_error = 0;


bool check_sharp() {
    constexpr int epsilon = 970;
    return vC > epsilon && (vR > epsilon || vL > epsilon);
}

void get_line_sensors() {
    vL = analogRead(left);
    vC = analogRead(middle);
    vR = analogRead(right);
}

void get_wall_sensors() {
    distL = readDistance(trigL, echoL);
    distC = readDistance(trigC, echoC);
    distR = readDistance(trigR, echoR);
}

void take_sharp() {
    int direction = vL > vR ? -1 : 1;
    Serial.println("Sharp turn");
    rightMotor(100);
    leftMotor(100);
    delay(400);
    rightMotor(0);
    leftMotor(0);
    delay(2000);
    rightMotor(70 * direction);
    leftMotor(-70 * direction);
    delay(1000);
    rightMotor(0);
    leftMotor(0);
    delay(2000);
    get_line_sensors();
    Serial.println("Sharp turn done");
    Serial.println(vC);
}

bool check_line() {
    int sum = vL + vC + vR;
    return sum != 0;
}

bool check_wall() {
    return distL + distR + distC != 150;
}

void follow_line() {
    int sum = vL + vC + vR;
    
    float error = (vR * 100.0 - vL * 100.0) / sum;    
    int correction = (lineKp * error) + (lineKd * (error - line_last_error));
    line_last_error =
     error;

    leftMotor(lineSpeed - correction);
    rightMotor(lineSpeed + correction);
}

void follow_wall() {
    float error = distL - distR;
    
    float correction = (wallKp * error) + (wallKd * (error - wall_last_error));
    wall_last_error = error;

    correction = constexprrain(correction, -25, 25); 

    leftMotor(wallSpeed + correction);
    rightMotor(wallSpeed - correction);
}

void loop() {
    get_line_sensors();

    if(check_line()) {
        if(check_sharp()) {
            take_sharp();
            return;
        }

        follow_line();
        return;
    }

    get_wall_sensors();

    if(check_wall()) {
        follow_wall();
        return;
    }
}
