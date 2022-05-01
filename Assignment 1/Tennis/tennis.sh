#!/bin/bash

SCORE1=50
SCORE2=50
GUESS1=0
GUESS2=0
BALL_LOCATION=0
END_GAME=0
TRUE=1
WINNER=0
FIRST_TIME=1
MIDDLEROW=" |       |       O       |       | "
# define global strings
print_board() { 
    clear
    SCORE1=$(($SCORE1 - $GUESS1))
    SCORE2=$(($SCORE2 - $GUESS2))
    echo " Player 1: {$SCORE1}         Player 2: ($SCORE2) "
    echo " --------------------------------- "
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    echo "$MIDDLEROW"
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    echo " --------------------------------- "
}

print_guess() {
    echo "       Player 1 played: ${GUESS1}"
    echo -n "       Player 2 played: ${GUESS2}"
    echo ""
    echo ""
}




# get p1 guess
read_p1_guess() {
    while [ $TRUE ];
    do
        echo "PLAYER 1 PICK A NUMBER: "
        read -s GUESS1
        if [[ ! $GUESS1 =~ ^[0-9]+$ ]] || [ $GUESS1 -lt 0 ] || [ $GUESS1 -gt $SCORE1 ];
            then
                echo "NOT A VALID MOVE !"
        else
                break
        fi
    done
}
# get p1 guess
read_p2_guess() {
    while [ $TRUE ];
    do
        echo "PLAYER 2 PICK A NUMBER: "
        read -s GUESS2
        if [[ ! $GUESS2 =~ ^[0-9]+$ ]] || [ $GUESS2 -lt 0 ] || [ $GUESS2 -gt $SCORE2 ];
            then
                echo "NOT A VALID MOVE !"
        else
                break
        fi
    done
}

check_for_winner() {
    if [ $BALL_LOCATION == 3 ];
    then
        WINNER=1
        END_GAME=1
    elif [ $BALL_LOCATION == -3 ];
    then
        WINNER=2
        END_GAME=1
    elif [ $SCORE1 == 0 ] && [ $SCORE2 -gt 0 ];
    then
        WINNER=2
        END_GAME=1
    elif [ $SCORE1 -gt 0 ] && [ $SCORE2 == 0 ];
    then
        WINNER=1
        END_GAME=1
    elif [ $SCORE1 -gt 0 ] && [ $SCORE2 == 0 ];
    then
        if [ $BALL_LOCATION -gt 0 ];
        then
            WINNER=2
            END_GAME=1
        else
            WINNER=1
            END_GAME=1
        fi
    fi
}

calculate_ball_locations() {
    # determine winner
    winner=$($GUESS1-$GUESS2)
    if [ winner -ge 0 ];
    then
        #p1 is the winner of the draw
        decrese_ball_location
    else # p2 is winner of the draw
        increase_ball_location
    fi
}

decrese_ball_location() {
    BALL_LOCATION=$($BALL_LOCATION - 1)
   case $BALL_LOCATION in
       1)
            MIDDLEROW=$(" |       |   O   #       |       | ")    
       ;;
       0 | -1)
            MIDDLEROW=$(" |       |       #   O   |       | ")
       ;;
       -2)
           MIDDLEROW=$()
       ;;
   esac
   
}

# print_board
# read_p1_guess
# read_p2_guess
# check_for_winner

#main

while [ $TRUE ]; 
do
    print_board
    if [ $FIRST_TIME == 0 ];
    then
        print_guess
    else
        FIRST_TIME=0
    fi
    print_guess
    calculate_ball_locations
done
