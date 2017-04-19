#/usr/bin/python

import pygame, random
from scipy.spatial import ConvexHull
from lib.util import extract_features, predict
from lib.constants import SHAPE_ARRAY


arr = []
feature_vector = []

def roundline(srf, color, start, end, radius=1):
    dx = end[0]-start[0]
    dy = end[1]-start[1]
    distance = max(abs(dx), abs(dy))
    for i in range(distance):
        x = int( start[0]+float(i)/distance*dx)
        y = int( start[1]+float(i)/distance*dy)
        pygame.draw.circle(srf, color, (x, y), radius)
def main():
    screen = pygame.display.set_mode((800,600))
    draw_on = False
    last_pos = (0, 0)
    color = (255, 128, 0)
    radius = 10
    try:
        while True:
            e = pygame.event.wait()
            if e.type == pygame.QUIT:
                raise StopIteration
            if e.type == pygame.MOUSEBUTTONDOWN:
                # color = (random.randrange(256), random.randrange(256), random.randrange(256))
                color = (255,255,255)
                pygame.draw.circle(screen, color, e.pos, radius)
                draw_on = True
            if e.type == pygame.MOUSEBUTTONUP:
                draw_on = False
                f = extract_features(arr)
                break
            if e.type == pygame.MOUSEMOTION:
                if draw_on:
                    pygame.draw.circle(screen, color, e.pos, radius)
                    roundline(screen, color, e.pos, last_pos,  radius)
                    arr.append(e.pos)
                last_pos = e.pos
            pygame.display.flip()
        pre = predict(f)[0]
        print SHAPE_ARRAY[int(pre)]

    except StopIteration:
        pass


    pygame.quit()

if __name__ == "__main__":
    main()
