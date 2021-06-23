#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <assert.h>


#include "ai.h"
#include "utils.h"
#include "priority_queue.h"


struct heap h;

float get_reward( node_t* n );

/**
 * Function called by pacman.c
*/
void initialize_ai(){
	heap_init(&h);
}

/**
 * function to copy a src into a dst state
*/
void copy_state(state_t* dst, state_t* src){
	//Location of Ghosts and Pacman
	memcpy( dst->Loc, src->Loc, 5*2*sizeof(int) );

    //Direction of Ghosts and Pacman
	memcpy( dst->Dir, src->Dir, 5*2*sizeof(int) );

    //Default location in case Pacman/Ghosts die
	memcpy( dst->StartingPoints, src->StartingPoints, 5*2*sizeof(int) );

    //Check for invincibility
    dst->Invincible = src->Invincible;
    
    //Number of pellets left in level
    dst->Food = src->Food;
    
    //Main level array
	memcpy( dst->Level, src->Level, 29*28*sizeof(int) );

    //What level number are we on?
    dst->LevelNumber = src->LevelNumber;
    
    //Keep track of how many points to give for eating ghosts
    dst->GhostsInARow = src->GhostsInARow;

    //How long left for invincibility
    dst->tleft = src->tleft;

    //Initial points
    dst->Points = src->Points;

    //Remiaining Lives
    dst->Lives = src->Lives;   

}

node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->priority = 0;
	new_n->depth = 0;
	new_n->num_childs = 0;
	copy_state(&(new_n->state), init_state);
	new_n->acc_reward =  get_reward( new_n );
	return new_n;
	
}


float heuristic( node_t* n ){
	float h = 0;
    float i = 0;
    float l = 0;
    float g = 0;
	
	//FILL IN MISSING CODE
    if(n->state.Invincible){
        i = 10;
    }
    if(n->parent){
        if(n->state.Lives < n->parent->state.Lives){
            l = 10;
        }
    }
    if(n->state.Lives == 0){
        g = 100;
    }

    h = i - l - g;
    
	return h;
}

float get_reward ( node_t* n ){
	float reward = 0;
	
	//FILL IN MISSING CODE
    reward = heuristic(n) + n->state.Points;
    
    if(n->parent){
        reward -= n->parent->state.Points;
    }
	float discount = pow(0.99,n->depth);
   	
	return discount * reward;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
bool applyAction(node_t* n, node_t* new_node, move_t action ){

	bool changed_dir = false;

    //FILL IN MISSING CODE
    copy_state(&(new_node->state), &(n->state));
    new_node->parent = n;
    new_node->priority = n->priority - 1;
    new_node->depth = n->depth + 1;
    new_node->move = action;
    new_node->num_childs = 0;
    changed_dir = execute_move_t( &((*new_node).state), action );
    new_node->acc_reward = get_reward(new_node);
    
	return changed_dir;

}

void propagateBackScoreToFirstAction(node_t* n, propagation_t propagation){
    float acc_reward = 0;
    acc_reward = n->acc_reward;
    n->parent->num_childs++;
    if(propagation == 0){
        while(n->parent){
            n = n->parent;
            acc_reward += n->acc_reward;
            if(acc_reward > n->acc_reward){
                n->acc_reward = acc_reward;
            }
        }
    }
    else{
        while(n->parent){
            n = n->parent;
            acc_reward = ((n->acc_reward * (n->num_childs-1)) + acc_reward) / n->num_childs;
            n->acc_reward = acc_reward;
        }
    }
}


/**
 * Find best action by building all possible paths up to budget
 * and back propagate using either max or avg
 */

move_t get_next_move( state_t init_state, int budget, propagation_t propagation, char* stats ){
	move_t best_action = rand() % 4;

	float best_action_score[4];
	for(unsigned i = 0; i < 4; i++)
	    best_action_score[i] = INT_MIN;

	unsigned generated_nodes = 0;
	unsigned expanded_nodes = 0;
	unsigned max_depth = 0;
	


	//Add the initial node
	node_t* n = create_init_node( &init_state );
    node_t** explored;
    explored = (node_t **)calloc(budget, sizeof(node_t));
    
	//Use the max heap API provided in priority_queue.h
	heap_push(&h,n);
	
	//FILL IN THE GRAPH ALGORITHM
    bool changedDir = false;
    
    
	while(h.count != 0){
        n = heap_delete(&h);
        //explore n
        if(expanded_nodes != 0){
            explored[expanded_nodes-1] = n;
        }
        for(int i = left; i <= down; i++ ){
            if(expanded_nodes <= budget){
                node_t* newNode = (node_t *)malloc(sizeof(node_t));
                changedDir = applyAction(n, newNode, i);
                generated_nodes++;
                if(changedDir){
                    propagateBackScoreToFirstAction(newNode, propagation);
                    if(n->state.Lives > newNode->state.Lives){
                        free(newNode);
                    }
                    else{
                        heap_push(&h, newNode);
                    }
                }
            }
        }
        expanded_nodes++;
    }
    
    float best_acc_reward = 0;
    int i = 0;
    while(explored[i]){
        if(explored[i]->depth > max_depth){
            max_depth = explored[i]->depth;
        }
        if(explored[i]->depth > 1){
        }
        else{
            if(explored[i]->acc_reward > best_acc_reward){
                best_action = explored[i]->move;
                best_acc_reward = explored[i]->acc_reward;
                //printf("Best_Acc_Reward = %f\n", best_acc_reward);
            }
            best_action_score[explored[i]->move] = explored[i]->acc_reward;
        }
        i++;
    }
    
    for(int i = 0; i < budget; i++){
        free(explored[i]);
    }
	
    free(explored);
    
	sprintf(stats, "Max Depth: %d Expanded nodes: %d  Generated nodes: %d\n",max_depth,expanded_nodes,generated_nodes);
	
	if(best_action == left)
		sprintf(stats, "%sSelected action: Left\n",stats);
	if(best_action == right)
		sprintf(stats, "%sSelected action: Right\n",stats);
	if(best_action == up)
		sprintf(stats, "%sSelected action: Up\n",stats);
	if(best_action == down)
		sprintf(stats, "%sSelected action: Down\n",stats);

	sprintf(stats, "%sScore Left %f Right %f Up %f Down %f",stats,best_action_score[left],best_action_score[right],best_action_score[up],best_action_score[down]);
	return best_action;
}

