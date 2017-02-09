import numpy as np
from PIL import Image

# Size of the bare = 16
# Size of the ball = 4*2

mean_ball = np.array([255, 228, 197])
mean_player = np.array([92, 185, 94])
mean_opponent = np.array([212, 130, 74])
mean_background = np.array([143, 72, 16])


def distance(color1, color2):
    return np.sum(np.absolute(color1 - color2))


def fill_position_distribution_1d(array_to_fill, initial_array, special_color):
    for i in range(160):
        if distance(initial_array[i], special_color) < distance(initial_array[i], mean_background):
            array_to_fill[i] = 1
    return


def fill_position_distribution_2d(array_to_fill, initial_array, special_color):
    for i in range(160):
        for j in range(160):
            if distance(initial_array[i, j, :], special_color) < distance(initial_array[i, j, :], mean_background):
                array_to_fill[i, j] = 1
    return


def process(image):
    # Prepare data
    ball_area = image.crop((0, 34, 160, 194))  # Size : 160 * 160
    opponent_area = image.crop((17, 34, 18, 194))  # Size : 1 * 160
    player_area = image.crop((141, 34, 142, 194))  # Size : 1 * 160
    player_distribution = np.zeros(160, dtype='B')
    opponent_distribution = np.zeros(160, dtype='B')
    ball_distribution = np.zeros((160, 160), dtype='B')
    
    # Find the positions distributions
    fill_position_distribution_1d(player_distribution, np.array(player_area)[:, 0, :], mean_player)
    fill_position_distribution_1d(opponent_distribution, np.array(opponent_area)[:, 0, :], mean_opponent)
    fill_position_distribution_2d(ball_distribution, np.array(ball_area), mean_ball)

    # Find the positions
    y_b = np.argmax(np.sum(ball_distribution, 1))
    x_b = np.argmax(np.sum(ball_distribution, 0))
    
    return x_b, y_b, player_distribution, opponent_distribution
    
    
    
    
    
    
    
    
    
