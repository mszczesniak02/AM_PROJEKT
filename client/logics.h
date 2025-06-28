#ifndef LOGICS_H
#define LOGICS_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "amcom.h"
#include "amcom_packets.h"
#include "structures.h"




#define EPSILON_DISTANCE    40.0f       // minimal distance for compairison if 2 objects 
#define EPSILON_ANGLE       0.4f        // angle in radians, determines minimal distance


#define PI                  3.141592
#define BYPASS_ARG          255         // used once, value too big .. coś tam

#define REQ_RESP_LOG        0           // show TCP connection log -> each packet send and recv
#define ALGORITHM_DATA_LOG  0           // show calculated cluster and nextList data

/**
 * \brief Finds the object described by the parameters. Decides on the head based on the type of an object.
 * \param type AMCOM_ObjectState type (0-player, 1-transistor,2-spark,3-glue)
 * \param num AMCOM_ObjectState number of an element
 * 
 */
AMCOM_ObjectState * findObject( uint8_t type, uint16_t num);


/**
 * \brief Finds the object described by the parameters. Decides on the head based on the type of an object.
 * \param state AMCOM_ObjectState array with data to fetch
 * \param count amount of AMCOM_ObjectState packets to fetch
 * 
 */
void fetchObjects(AMCOM_ObjectState * state, uint8_t count);

/**
 * \brief  Get (x,y) coordinates from (-500,500) to (0, 1000)
    Doesn't change the object itself, puts the transposed data into func parameters 'x' and 'y'
 * \param object AMCOM_ObjectState object
 * \param x return value 
 * \param y return value  

 */
void Transpose(AMCOM_ObjectState * object, float * x, float * y);


/**
 * @brief Calculates the Euclidean distance between two game objects.
 * @param object_origin The starting object (e.g., the player).
 * @param object_dest The destination object (e.g., food, another player).
 * @return The calculated distance as a float.
 */
float GetDistance(AMCOM_ObjectState * object_origin,AMCOM_ObjectState * object_dest );


/**
 * @brief Calculates the absolute angle in radians from an origin object to a destination object.
 * @note The angle is calculated using atan2f, providing a result in the range [-PI, PI].
 * @param object_origin The starting object (e.g., the player).
 * @param object_dest The destination object.
 * @return The calculated angle in radians.
 */
float GetAbsoluteAngle(AMCOM_ObjectState * object_origin, AMCOM_ObjectState * object_dest);


float Move(AMCOM_ObjectState * player, AMCOM_ObjectState * food);


/**
 * @brief Checks if a spark object is in the direct path between the player and a food object.
 * @note An obstacle is considered "in the way" if it's closer than the food and within a
 *       predefined angle tolerance (EPSILON_ANGLE).
 * @param player The player's object state.
 * @param food The target food's object state.
 * @return 1 if a spark is in the way, 0 otherwise.
 */
uint8_t SparkInWay(AMCOM_ObjectState * player, AMCOM_ObjectState * food);



/**
 * \brief  Function used as qsort comparator.
 */
int ClusterCompair(const void* a, const void* b);

/**
 * \brief  Quick sort the lists of RelativePostion_t node.
 */
void sortClustersPosisions(RelativePosition_t * node[], uint8_t count);


/**
 * \brief  Print the whole RelativePostion list.
 */
void printClusterNode(RelativePosition_t* node[], uint8_t count);


/**
 * \brief  Reverse stack order (reverse Cluster helper)
 */
void reverseElementsList(cluster_element_t** head_ref);

/**
 * \brief  Reverse stack order of a cluster_list
 */
void reverseClusterList(cluster_list_t** list_head_ref);

/**
 * \brief clear elements of a clusterElements (stack)
 */
void clearClusterElements(cluster_element_t** head);

/**
 * \brief clear elements of a ClusterList (stack with ClusterElements as data)
 */
void clearClusterList(cluster_list_t** head);

/**
 * @brief Main function to find clusters of food objects based on proximity.
 * @note This function uses a simple greedy algorithm. It iterates through sorted food items,
 *       and for each unvisited item, it creates a new cluster, adding all other nearby
 *       unvisited items to it.
 * @param nodes An array of pointers to RelativePosition_t structures, sorted by distance to the player.
 * @param count The number of items in the nodes array.
 */
void getClusters(RelativePosition_t* nodes[], uint8_t count);


/**
 * @brief Orchestrates the entire process of planning the route for eating food.
 * @note This function:
 *       1. Calculates the relative position of all food items to the player.
 *       2. Sorts them by distance.
 *       3. Calls getClusters() to group them.
 *       4. The final plan is stored in the global GameDetails->next_stop array.
 * @param head A pointer to the head of the food list (head_t).
 * @param player A pointer to the player's object state.
 * @param count The total number of food items.
 */
void calculateClustersPosition(ll_t** head, AMCOM_ObjectState * player, uint8_t count );

/**
 * \brief  Adds the food clusters into the GameDetails global var.
 */
void NextStop_addClusters(cluster_list_t** head);


/**
 * @brief Checks if the player is close enough to the current food target to consider it "eaten".
 * @note The distance check uses the EPSILON_DISTANCE constant. If the food is reached,
 *       its corresponding flag in `next_stop_visited` is set to 1.
 * @param player The player's object state.
 * @return 1 if the food is reached, 0 otherwise.
 */
uint8_t foodReached(AMCOM_ObjectState * player);



/**
 * \brief  check if element is in  a list
 * \param element element to look for
 * \param arr list to search
 * \param count num of elements in list
 */
uint8_t inList(uint8_t element, const uint8_t arr[], uint8_t count);

/**
 * @brief Finds the next available food target that has not been eaten or temporarily excluded.
 * @note This function iterates through the `next_stop` plan and selects the first target that
 *       is not marked as visited and is not in the `excluded_indices` array.
 * @param excluded_indices An array of indices to temporarily ignore (e.g., because they are blocked).
 * @param excluded_count The number of items in the `excluded_indices` array.
 * @param player The player's object state.
 * @return 1 if a new target was found and set, 0 if no available targets are left.
 */
uint8_t foodNext(const uint8_t excluded[], uint8_t count, AMCOM_ObjectState * player);



/**
 * @brief Adds a new node to the beginning of a linked list (pushes to the head).
 * @param head Double pointer to the head of the list. The head is updated to point to the new node.
 * @param o Pointer to the object state to be added.
 */
ll_t * createNode(AMCOM_ObjectState * o);

/**
 * \brief Push node into the head element
 *  
 * @param head the first element of a stack
 * @param o Structure describing the state of a single game object 
 *               
 */
void pushNode(ll_t** head, AMCOM_ObjectState * o );




/**
 * \brief initializer of the GameDetails structure.
 */
void GameDetails_init(void);

/**
 * \brief initializer of the NextStop_t structure. Holds the desired foods list.
 */
void NextStop_Init(void);

/**
 * \brief free memory of initialized next_stop structure.
 */
void NextStop_Free(void);

/**
 * @brief Frees all nodes in a linked list and the data they contain.
 * @note After clearing, the head pointer is set to NULL to prevent dangling pointers.
 * @param head Double pointer to the head of the list to be cleared.
 */
void clearNode(ll_t ** head);


/**
 * \brief Prints details of an AMCOM_ObjectState.
 *  
 * @param o Structure describing the state of a single game object 
 *               
 */
void PrintObject(AMCOM_ObjectState * o);




/**
 * \brief  Initializes the structure RelativePostion_t.
 * \param state AMCOM_ObjectState object to initiate with
 */
RelativePosition_t* RelativePosition_Init(AMCOM_ObjectState* o);

/**
 * \brief  Initializes the structure RelativePostion_t.
 * \param o RelativePosition element
 */
void RelativePosition_free(RelativePosition_t* o);


/**
 * \brief  Prints single instance of RelativePosition_t.
 * \param o RelativePosition element
 */
void RelativePosition_Print(RelativePosition_t* o);





/**
 * \brief  Counts and returns the amount of objects in a stack structure.
 * \param head stack head, first element
 */
uint8_t CountNodes(ll_t** head);




/**
 * \brief  Create an instance of a stack for cluster_element_t.
 */
cluster_element_t* createClusterElement(next_stop_t * data);

/**
 * \brief  push Stack structure into the head, having data as next_stop_t.
 * \param head head element, first alement
 * \param o object to initiate as data
 */
void pushClusterElement(cluster_element_t** head, next_stop_t * o );


/**
 * \brief  Stack structure with cluster_element_t as data. Outer list, that holds innner list, creates new instance
 */
cluster_list_t * createCluster(cluster_element_t * o);


/**
 * \brief  Push onto stack the cluster structure
 */
void pushCluster(cluster_list_t** head, cluster_element_t * o );


/**
 * \brief  Prints the whole cluster
 */
void printCluster(cluster_list_t** head);


/**
 * @brief The main packet handler for processing all messages from the server.
 * @note This is the core of the client's logic. It handles game state updates,
 *       responds to server requests, and implements the player's AI for movement decisions.
 *       It contains the state machine for switching between "foodie mode" and "predator mode".
 * @param packet The incoming packet from the server.
 * @param userContext A void pointer to user-defined context, used here to pass the client socket.
 */
void amPacketHandler(const AMCOM_Packet* packet, void* userContext);






#endif