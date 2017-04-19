Current repository consists of a Shape Recognition model.
It takes input as a coordinates based on the mouse movement while drawing and predicts the shape.
Currently it's modelled to detect circle, triangle, square and star.
It used k-nn algorithm to find the best fit match.
Features consists of following:
1. perimeter^2/area of closed figure
2. area_traingle/area where area_triangle is the largest area of triangle enclosed by the given fugure
3. perimeter_rect/perimeter where perimeter_rect denoted the perimeter of the largest rectange enclosed by the given convexHull
4. area^2/(area_traingle*area_rect)

