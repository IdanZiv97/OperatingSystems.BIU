#!/bin/bash
#Idan Ziv 318175197
SCORE1=50
SCORE2=50
GUESS1=0
GUESS2=0
BALL_LOCATION=0
END_GAME=0
TRUE=1
# define global strings
print_board() { 
    # SCORE1=$(($SCORE1 - $GUESS1))
    # SCORE2=$(($SCORE2 - $GUESS2))
    echo " Player 1: $SCORE1         Player 2: $SCORE2 "
    echo " --------------------------------- "
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    print_middle_row
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

print_middle_row() {
    case $BALL_LOCATION in
        -3)
            echo "O|       |       #       |       | "
        ;;
        -2)
            echo " |   O   |       #       |       | "
        ;;
        -1)
            echo " |       |   O   #       |       | "
        ;;
        0)
            echo " |       |       O       |       | "
        ;;
        1)
            echo " |       |       #   O   |       | "
        ;;
        2)
            echo " |       |       #       |   O   | "
        ;;
        3)
            echo " |       |       #       |       |O"
        ;;
    esac
    
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
# try to calculate the score here
check_for_winner() {
    if [ $GUESS1 -eq 50 ] && [ $GUESS2 -eq 50 ] && [ $BALL_LOCATION -eq 0 ];
    then
        echo "IT'S A DRAW !"
        END_GAME=1
    elif [ $BALL_LOCATION == 3 ];
    then
        print_board
        print_guess
        echo "PLAYER 1 WINS !"
        END_GAME=1
    elif [ $BALL_LOCATION == -3 ];
    then
        print_board
        print_guess
        echo "PLAYER 2 WINS !"
        END_GAME=1
    elif [ $SCORE1 -eq 0 ] && [ $SCORE2 -gt 0 ];
    then
        print_board
        print_guess
        echo "PLAYER 2 WINS !"
        END_GAME=1
    elif [ $SCORE1 -gt 0 ] && [ $SCORE2 -eq 0 ];
    then
        print_board
        print_guess
        echo "PLAYER 1 WINS !"
        END_GAME=1
    elif [ $SCORE1 -gt 0 ] && [ $SCORE2 -eq 0 ];
    then
        if [ $BALL_LOCATION -gt 0 ];
        then
            print_board
            print_guess
            echo "PLAYER 2 WINS !"
            END_GAME=1
        else
            print_board
            print_guess
            echo "PLAYER 1 WINS !"
            END_GAME=1
        fi
    fi
}

calculate_ball_locations() {
    # determine winner
    winner=$(($GUESS1-$GUESS2))
    if [ $winner -eq 0 ];
    then
        check_for_winner
    elif [ $winner -ge 0 ];
    then
        #p1 is the winner of the draw
        p1_won_guess
        check_for_winner
    else # p2 is winner of the draw
        p2_won_guess
        check_for_winner
    fi
}

p1_won_guess() {
    # the ball is in p1's court
    if [ $BALL_LOCATION -lt 0 ];
    then
        BALL_LOCATION=1
    else
        BALL_LOCATION=$(($BALL_LOCATION+1))
    fi
}

p2_won_guess() {
    # the ball is in p2's court
    if [ $BALL_LOCATION -gt 0 ];
    then
        BALL_LOCATION=-1
    else
        BALL_LOCATION=$(($BALL_LOCATION-1))
    fi
}

# main
print_board
while [ $TRUE ]; 
do
    read_p1_guess
    read_p2_guess
    SCORE1=$(($SCORE1 - $GUESS1))
    SCORE2=$(($SCORE2 - $GUESS2))
    calculate_ball_locations
    # check_for_winner
    if [ $END_GAME -eq 1 ];
    then
        break
    fi
    print_board
    print_guess
done
