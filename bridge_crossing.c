#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//Global variables
int firstVehicleHasCrossed = 0;

typedef struct waitinglist
{
	int vehicle_id;
	int vehicle_type;
	int direction;
	struct  waitinglist *next;
} waitingvehiclelist;

typedef struct movinglist
{
	int vehicle_id;
	int vehicle_type;
	int direction;
	struct  movinglist *next;
} movingvehiclelist;

typedef struct pmstr
{
	int vehicle_id;
	int vehicle_type;
	int direction;
} pmstr_t;

struct waitinglist *pw; //pointer to the waiting list
struct movinglist *pm;  //pointer to the moving list

int waitingcarsouth, waitingcarnorth, waitingtrucksouth, waitingtrucknorth;
int movingcar, movingtruck;
int currentmovingdir, previousmovingdir; //0:north, 1: south
pthread_mutex_t lock;
pthread_cond_t TruckNorthMovable, TruckSouthMovable, CarNorthMovable, CarSouthMovable;



void *vehicle_routine(void *pmstrpara); //vehicle_type: 0 for truck, 1 for car;
                                           //direction: 0 for north, 1 for south;
void vehicle_arrival(pmstr_t *pmstrpara);
void waitinglistinsert(int vehicle_id,int vehicle_type, int direction);
void waitinglistdelete(int vehicle_id);
void movinglistinsert(int vehicle_id, int vehicle_type, int direction);
void movinglistdelete(int vehicle_id);

void printmoving();
void printwaiting();


int main(int argc, char *argv[])
{
	int option;
	int i,j;
	float carprob;
	int vehicle_type, direction, vehicle_id;

	pthread_t vehicle[30];
	pmstr_t *pmthread;


	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&TruckNorthMovable, NULL);
	pthread_cond_init(&TruckSouthMovable, NULL);
	pthread_cond_init(&CarNorthMovable, NULL);
	pthread_cond_init(&CarSouthMovable, NULL);


	waitingcarnorth=0;
	waitingcarsouth=0;
	waitingtrucknorth=0;
	waitingtrucksouth=0;
	movingcar=0;
	movingtruck=0;
	option=-1;
	pw = NULL; //waiting list
	pm = NULL; //moving list

	fprintf(stderr,"***************************************************************\n");
	fprintf(stderr,"Please select one Schedules from the following six options:\n");
	fprintf(stderr,"1. 10 : DELAY(10) : 10\n");
	fprintf(stderr,"   car/truck probability: [1.0, 0.0]\n");
	fprintf(stderr,"2. 10 : DELAY(10) : 10\n");
	fprintf(stderr,"   car/truck probability: [0.0, 1.0]\n");
	fprintf(stderr,"3. 20\n");
	fprintf(stderr,"   car/truck probability: [0.65, 0.35]\n");
	fprintf(stderr,"4. 10 : DELAY(25) : 10 : DELAY(25) : 10\n");
	fprintf(stderr,"   car/truck probability: [0.5, 0.5]\n");
	fprintf(stderr,"5. 10 : DELAY(3) : 10 : DELAY(10): 10\n");
	fprintf(stderr,"   car/truck probability: [0.65, 0.35]\n");
	fprintf(stderr,"6. 20 : DELAY(15) : 10\n");
	fprintf(stderr,"   car/truck probability: [0.75, 0.25]\n");
	do
	{
		fprintf(stderr,"\nPlease select [1-6]:");
		option = atoi(argv[1]);
		//scanf("%d", &option);
	}while((option<0) || (option>6));

	fprintf(stderr,"***************************************************************\n");

	switch (option)
	{
		case 1: carprob = 1;    break;
		case 2: carprob = 0;    break;
		case 3: carprob = 0.65; break;
		case 4: carprob = 0.5;  break;
		case 5: carprob = 0.65; break;
		case 6: carprob = 0.75; break;
		default: carprob = 0.5;
	}

	srand((unsigned int)time((time_t *)NULL));
	
	//schedule 1:
	// 10 vehicles (10 pause) 10 more vehicles
	// only cars

	if(option==1) // 20 vehicles
	{
		pmstr_t args [20];  //setting up an array of arguments to store all values of vehicles
        pthread_mutex_lock(&lock);
		for (j=0; j<=9; j++) // the first ten vehicles arrive
		{

            //call rand() to decide vehicle type and direction
            
            
            float r = (rand() % 100) / 100;
            int direct = rand() % 2;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine, (void *)&args[j]);

            
		}
		pthread_mutex_unlock(&lock);


	    sleep(10);//delay (10)



		pthread_mutex_lock(&lock);
		for (j=10; j<=19; j++) // the second 10 vehicles arrive
		{
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;   


            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);
		}
		pthread_mutex_unlock(&lock);


		for (j=0; j<=19; j++)
			pthread_join(vehicle[j], NULL);

	} //end of option 1

	//schedule 2: 
	// 10 vehicles (10 pause) 10 more vehicles
	// only trucks

	else if(option==2) // 20 vehicles
	{
		pmstr_t args [20];
        pthread_mutex_lock(&lock);
		for (j=0; j<=9; j++) // first ten vehicles arrive
		{


            //call rand() to decide vehicle type and direction
            
            
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;

			args[j].vehicle_type = 0;
     

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);

            
		}
		pthread_mutex_unlock(&lock);


	    sleep(10);//delay (10)



		pthread_mutex_lock(&lock);
		for (j=10; j<=19; j++) // second ten vehicles arrive
		{
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);
		}
		pthread_mutex_unlock(&lock);


		for (j=0; j<=19; j++)
			pthread_join(vehicle[j], NULL);
} //end of option 2

	//schedule 3:
	// 20 vehicles () 
	// 65% cars 35% chance trucks

	else if(option==3) // 20 vehicles
	{

		pmstr_t args [20];
        pthread_mutex_lock(&lock);
		for (j=0; j<=19; j++) //spawn all twenty vehicles
		{


            //call rand() to decide vehicle type and direction
            
            
			float r = (rand()%100); //find a random number 0 through 99
			r = r/100; //change it to 0.0 through 0.99
			
			int direct;
                        
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);

            
		}
		
		pthread_mutex_unlock(&lock);


		for (j=0; j<=19; j++)
			pthread_join(vehicle[j], NULL);
	} // end of option3

	//schedule 4:
	// 10 vehicles (25 pause) 10 vehicles (25 pause) 10 vehicles
	// 50% cars 50% trucks

	else if(option==4) // 30 vehicles
	{
		pmstr_t args [30];
        pthread_mutex_lock(&lock);
		for (j=0; j<=9; j++) //spawn first 10 vehicles
		{


            //call rand() to decide vehicle type and direction
            
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);

            
		}
		pthread_mutex_unlock(&lock);


	    sleep(25);//delay (25)



		pthread_mutex_lock(&lock);
		for (j=10; j<=19; j++) //send in second 10 vehicles
		{
			
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);
		}
		pthread_mutex_unlock(&lock);

	    sleep(25);//delay (25)



		pthread_mutex_lock(&lock);
		for (j=20; j<=29; j++) //send in the final 10 vehicles
		{
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);
		}
		pthread_mutex_unlock(&lock);

		for (j=0; j<=29; j++)
			pthread_join(vehicle[j], NULL);

	} // end of option4

	//schedule 5:
	// 10 vehicles (3 pause) 10 vehicles (10 pause) 10 vehicles
	// 65% cars 35% trucks

	else if(option==5) // 30 vehicles
	{

		pmstr_t args [30];
        pthread_mutex_lock(&lock);
		for (j=0; j<=9; j++) //send in the first 10 vehicles
		{


            //call rand() to decide vehicle type and direction
            
            
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);

            
		}
		pthread_mutex_unlock(&lock);


	    sleep(3);//delay (3)



		pthread_mutex_lock(&lock);
		for (j=10; j<=19; j++) // the second 10 vehicles arrive
		{
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);
		}
		pthread_mutex_unlock(&lock);

	    sleep(10);//delay (10)



		pthread_mutex_lock(&lock);
		for (j=20; j<=29; j++) //send in the final 10 vehicles
		{
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);
		}
		pthread_mutex_unlock(&lock);

		for (j=0; j<=29; j++)
			pthread_join(vehicle[j], NULL);
	} // end of option5

	//schedule 6:
	// 20 vehicles (pause 15) 10 vehicles
	// 75% car 25% truck

	else //option6: 30 vehicles
	{

		pmstr_t args [30];
        pthread_mutex_lock(&lock);
		for (j=0; j<=19; j++) //send in the first 20 vehicles
		{


            //call rand() to decide vehicle type and direction
            
            
            
			float r = (rand()%100);
			r = r/100;
			
			int direct;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);

            
		}
		pthread_mutex_unlock(&lock);


	    sleep(15);//delay (15)



		pthread_mutex_lock(&lock);
		for (j=20; j<=29; j++) //send in the final 10 vehicles
		{
			double r = (rand() % 100) / 100;
            int direct = rand() % 2;
            
            //generate pmstr_t struct to save the vehicle type, direction, and id
            
            direct = rand() % 2;
            args[j].vehicle_id = j;
            args[j].direction = direct;
            if(r <= carprob){
                args[j].vehicle_type = 1;
            }
            else args[j].vehicle_type = 0;            

            //call vehicle_arrival()
            vehicle_arrival(&args[j]);
            
            //create a pthread to represent the vehicle, vehicle_routine() is the start function of a pthread
            pthread_create(&vehicle[j], NULL, vehicle_routine , (void *)&args[j]);
		}
		pthread_mutex_unlock(&lock);

		for (j=0; j<=29; j++)
			pthread_join(vehicle[j], NULL);
	} // end of option6



	fprintf(stderr,"\nFinished execution.\n");



} // end of main function


void *vehicle_routine(void *pmstrpara_meth_arg)
{
	
	char *strdir;
	pmstr_t *pmstrpara = (pmstr_t *)pmstrpara_meth_arg;

	if (pmstrpara->vehicle_type) //car
	{
		firstVehicleHasCrossed = 0;
		pthread_mutex_lock(&lock);
		//Try to cross
	//Checking to see if the car cannot cross, matching conditions like moving car at max of three
	// or any moving trucks, or any waiting trucks
	// or moving car in different direction
	int cantCross = 	(movingcar == 3 || movingtruck != 0) ||
						(waitingtrucknorth != 0 || waitingtrucksouth != 0) ||
						(movingcar > 0 && pmstrpara->direction != currentmovingdir) ||
						(pmstrpara->direction == 1 && !firstVehicleHasCrossed);
		//while (this vehicle cannot cross) {
		while (cantCross){
			if(pmstrpara->direction == 0) 
				pthread_cond_wait(&CarNorthMovable, &lock);
			else pthread_cond_wait(&CarSouthMovable, &lock);
			
			if(pmstrpara->direction == 0) firstVehicleHasCrossed = 1;
		//     wait for proper moving signal

			
		cantCross = 	(movingcar == 3 || movingtruck != 0) ||
						(waitingtrucknorth != 0 || waitingtrucksouth != 0) ||
						(movingcar > 0 && pmstrpara->direction != currentmovingdir);
		}

		//Now begin accrossing
		if(firstVehicleHasCrossed || pmstrpara->direction == 0) {
			movinglistinsert(pmstrpara->vehicle_id, pmstrpara->vehicle_type, pmstrpara->direction);
			waitinglistdelete(pmstrpara->vehicle_id);
			firstVehicleHasCrossed = 1;
		

		//update global variables
		if (pmstrpara->direction) waitingcarsouth --;
		else waitingcarnorth --;
		movingcar++;

		//print out proper message
		printmoving();
		printwaiting();

		pthread_mutex_unlock(&lock);

		sleep(2);

		//The car is now leaving 
		pthread_mutex_lock(&lock);
		movinglistdelete(pmstrpara->vehicle_id);

		//update global variables
		previousmovingdir = currentmovingdir;
		movingcar--;

		//send out signals to wake up vehicle(s) accordingly
		if (movingcar ==0) {
			if (waitingtrucknorth > 0) {
				pthread_cond_signal(&TruckNorthMovable);
			}
			else if (waitingtrucksouth > 0) {
				pthread_cond_signal(&TruckSouthMovable); 
				}
			else if (waitingcarnorth > 0) {
				currentmovingdir = 0; //sets the direction to that of the oncoming car, required for 3 cars to join
				for(int i = movingcar; i < 3; i ++){
					pthread_cond_signal(&CarNorthMovable);
				} 	
			}
			else if (waitingcarsouth > 0) {
				currentmovingdir = 1; //sets the direction to that of the oncoming car, required for 3 cars to join
				//fprintf(stderr,"\n %d The program thinks there are %d moving cars.\n", waitingcarsouth, movingcar);
				for(int i = movingcar; i < 3; i ++){
					pthread_cond_signal(&CarSouthMovable);
				} 
			}
		}
		
    	fprintf(stderr,"\nCar #%d exited the bridge.\n", pmstrpara->vehicle_id);

		pthread_mutex_unlock(&lock);

		}
	}


	else //truck
	{
		pthread_mutex_lock(&lock);
		//Try to cross

		//setting cantCross to the conditions for the truck to not be able to cross
		//if there are 3 moving cars, any moving trucks, or a moving car in the opposite direction
		int cantCross = (movingcar == 3 || movingtruck != 0) ||
						(movingcar > 0 && pmstrpara->direction != currentmovingdir) ||
						(pmstrpara->direction == 1 && !firstVehicleHasCrossed);

		//while (this vehicle cannot cross) {
		while (cantCross){

			//fprintf(stderr,"\nid %d\n", pmstrpara->vehicle_id);
			//fprintf(stderr,"\ndirection %d\n", pmstrpara->direction);
			//fprintf(stderr,"\ntype %d\n", pmstrpara->vehicle_type);





			if(pmstrpara->direction == 0)
				pthread_cond_wait(&TruckNorthMovable, &lock);
			else pthread_cond_wait(&TruckSouthMovable, &lock);

			if(pmstrpara->direction == 0) firstVehicleHasCrossed = 1;
		
		//     wait for proper moving signal
		cantCross = (movingcar == 3 || movingtruck != 0) ||
		(movingcar > 0 && pmstrpara->direction != currentmovingdir);
		}


		//Now begin accrossing
		if(firstVehicleHasCrossed || pmstrpara->direction == 0) {
			firstVehicleHasCrossed = 1;
			movinglistinsert(pmstrpara->vehicle_id, pmstrpara->vehicle_type, pmstrpara->direction);
			waitinglistdelete(pmstrpara->vehicle_id);
		
		//update global variables
		if (pmstrpara->direction) waitingtrucksouth --;
		else waitingtrucknorth --;
		movingtruck++;

		//print out proper message
		printmoving();
		printwaiting();

		pthread_mutex_unlock(&lock);

		sleep(2);	//delay (2)

		//The truck is now leaving
		pthread_mutex_lock(&lock);
		movinglistdelete(pmstrpara->vehicle_id);

		//update global variables
		previousmovingdir = currentmovingdir;
		movingtruck --;

		//fprintf(stderr,"\nid %d direciton %d\n", pmstrpara->vehicle_id, pmstrpara->direction);

		//send out signals to wake up vehicle(s) accordingly
		if (movingtruck == 0) {
			
			if (waitingtrucknorth > 0 && previousmovingdir == 1) {
				currentmovingdir = 0; //sets the direction to that of the oncoming truck, required for alternation
				previousmovingdir = 0;
				pthread_cond_signal(&TruckNorthMovable);
			}
			else if (waitingtrucksouth > 0 && previousmovingdir == 0) {
				currentmovingdir = 1; //sets the direction to that of the oncoming truck, required for alternation
				previousmovingdir = 1;
				pthread_cond_signal(&TruckSouthMovable); 
				}
			else if (waitingtrucknorth > 0 ) {
				currentmovingdir = 0; //sets the direction to that of the oncoming truck, required for alternation
				previousmovingdir = 0;
				pthread_cond_signal(&TruckNorthMovable);
			}
			else if (waitingtrucksouth > 0 ) {
				currentmovingdir = 1; //sets the direction to that of the oncoming truck, required for alternation
				previousmovingdir = 1;
				pthread_cond_signal(&TruckSouthMovable); 
				}
			else if (waitingcarnorth > 0) {
				previousmovingdir = 0;
				currentmovingdir = 0; //sets the direction to that of the oncoming car, required for 3 cars to join
				for(int i = movingcar; i < 3; i ++){
					pthread_cond_signal(&CarNorthMovable);
				} 	
			}
			else if (waitingcarsouth > 0) {
				previousmovingdir = 1;
				currentmovingdir = 1; //sets the direction to that of the oncoming car, required for 3 cars to join
				for(int i = movingcar; i < 3; i ++){
					pthread_cond_signal(&CarSouthMovable);
				} 
			}
		}
		
		fprintf(stderr,"\nTruck #%d exited the bridge.\n", pmstrpara->vehicle_id);

		pthread_mutex_unlock(&lock);

		}
	}



} // end of thread routine


void vehicle_arrival(pmstr_t *pmstrpara)
{
	if(pmstrpara->vehicle_type)
	{
		if (pmstrpara->direction)
			waitingcarsouth++;
		else
			waitingcarnorth++;
	}
	else
	{
		if (pmstrpara->direction)
			waitingtrucksouth++;
		else
			waitingtrucknorth++;
	}
	waitinglistinsert(pmstrpara->vehicle_id,pmstrpara->vehicle_type,pmstrpara->direction);

	if(pmstrpara->vehicle_type)
	{
		if (pmstrpara->direction)
			fprintf(stderr,"\nCar #%d (southbound) arrived.\n",pmstrpara->vehicle_id);
		else
			fprintf(stderr,"\nCar #%d (northbound) arrived.\n",pmstrpara->vehicle_id);
	}
	else
	{
		if (pmstrpara->direction)
			fprintf(stderr,"\nTruck #%d (southbound) arrived.\n",pmstrpara->vehicle_id);
		else
			fprintf(stderr,"\nTruck #%d (northbound) arrived.\n",pmstrpara->vehicle_id);
	}
} // end of vehicle_arrival

void waitinglistinsert(int vehicle_id,int vehicle_type, int direction)
{
	struct waitinglist *p;
	p=(struct waitinglist *)malloc(sizeof(struct waitinglist));
	p->vehicle_id = vehicle_id;
	p->vehicle_type = vehicle_type;
	p->direction = direction;
	p->next = pw;
	pw = p;
}

void waitinglistdelete(int vehicle_id)
{
	struct waitinglist *p, *pprevious;
	p = pw;
	pprevious = p;
	while(p)
	{
		if ((p->vehicle_id)==(vehicle_id))
			break;
		else
		{
			pprevious = p;
			p=p->next;
		}
	}

	if(p==pw)
		pw=p->next;
	else
		pprevious->next=p->next;

	free(p);
}

void movinglistinsert(int vehicle_id, int vehicle_type, int direction)
{
	struct movinglist *p;
	p=(struct movinglist *)malloc(sizeof(struct movinglist));
	p->vehicle_id = vehicle_id;
	p->vehicle_type = vehicle_type;
	p->direction = direction;
	p->next = pm;
	pm = p;

}

void movinglistdelete(int vehicle_id)
{
	struct movinglist *p, *pprevious;
	p = pm;
	pprevious = p;
	while(p)
	{
		if ((p->vehicle_id)==(vehicle_id))
			break;
		else
		{
			pprevious = p;
			p=p->next;
		}
	}

	if(p==pm)
		pm=p->next;
	else
		pprevious->next=p->next;

	free(p);
}

void printmoving()
{
	struct movinglist *p;
	p = pm;
	fprintf(stderr,"Vehicles on the bridge: [");
	while(p)
	{
		if (p->vehicle_type)
			fprintf(stderr,"Car #%d,",p->vehicle_id);
		else
			fprintf(stderr,"Truck #%d,",p->vehicle_id);
		p=p->next;
	}

	fprintf(stderr,"]\n");
	fprintf(stderr,"Now %d cars are moving.\n", movingcar);
	fprintf(stderr,"Now %d trucks are moving.\n", movingtruck);
	//fprintf(stderr,"Current moving direction: %d.\n", currentmovingdir);

}
void printwaiting()
{
	struct waitinglist *p;
	p = pw;
	fprintf(stderr,"Waiting Vehicles (northbound): [");
	while(p)
	{
		if(p->direction==0)
		{
			if (p->vehicle_type)
		 		fprintf(stderr,"Car #%d,",p->vehicle_id);
			else
				fprintf(stderr,"Truck #%d,",p->vehicle_id);
		}
		p=p->next;
	}

	fprintf(stderr,"]\n");

	p = pw;
	fprintf(stderr,"Waiting Vehicles (Southbound): [");
	while(p)
	{
		if(p->direction)
		{
			if (p->vehicle_type)
		 		fprintf(stderr,"Car #%d,",p->vehicle_id);
			else
				fprintf(stderr,"Truck #%d,",p->vehicle_id);
		}
		p=p->next;
	}

	fprintf(stderr,"]\n");

	//fprintf(stderr,"Now %d cars (south) are waiting.\n", waitingcarsouth);
	//fprintf(stderr,"Now %d cars (north) are waiting.\n", waitingcarnorth);
	//fprintf(stderr,"Now %d trucks (south) are waiting.\n", waitingtrucksouth);
	//fprintf(stderr,"Now %d trucks (north) are waiting.\n", waitingtrucknorth);
}