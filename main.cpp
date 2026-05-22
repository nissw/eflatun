const int right  = A0;
const int middle = A1;
const int left   = A2;

const int trigR = 1; const int echoR = 0; 
const int trigC = 8;  const int echoC = 7;
const int trigL = 4;  const int echoL = 2;

const int m1a = 6;
const int m1b = 5;
const int m2a = 9;
const int m2b = 10;

float lineKp = 5.5; 
float lineKd = 1.5; 
float lineLastError = 0;

float wallKp = 4.0; 
float wallKd = 1.0;
float wallLastError = 0;

const int baseSpeed = 20; 

// --- EMA PARAMETRELERİ ---
float emaAlphaLine = 0.4; 
float emaAlphaWall = 0.5; 

float filteredLineError = 0; 
float filteredWallError = 0; 
// --------------------------

int vL, vR, vC, distL, distR, distC;

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
  hiz = constrain(hiz, -255, 255);
  if (hiz > 0) {
    digitalWrite(m1a, LOW); analogWrite(m1b, hiz);
  } else if (hiz < 0) {
    analogWrite(m1a, -hiz); digitalWrite(m1b, LOW);
  } else {
    digitalWrite(m1a, LOW); digitalWrite(m1b, LOW);
  }
}

void rightMotor(int hiz) {
  hiz = constrain(hiz, -255, 255);
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

bool check_sharp() {
    const int epsilon = 970;
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
    
    filteredLineError = 0;
    lineLastError = 0;
    
    Serial.println("Sharp turn done");
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
    if (sum == 0) sum = 1; 
    
    float raw_error = (vR * 100.0 - vL * 100.0) / sum; 
    
    filteredLineError = (emaAlphaLine * raw_error) + ((1.0 - emaAlphaLine) * filteredLineError);
    
    int correction = (lineKp * filteredLineError) + (lineKd * (filteredLineError - lineLastError));
    lineLastError = filteredLineError;

    leftMotor(baseSpeed - correction);
    rightMotor(baseSpeed + correction);
}

void follow_wall() {
    float raw_error = distL - distR;
    
    filteredWallError = (emaAlphaWall * raw_error) + ((1.0 - emaAlphaWall) * filteredWallError);
    
    float correction = (wallKp * filteredWallError) + (wallKd * (filteredWallError - wallLastError));
    wallLastError = filteredWallError;

    correction = constrain(correction, -25, 25); 

    leftMotor(baseSpeed + correction);
    rightMotor(baseSpeed - correction);
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
