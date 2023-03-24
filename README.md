# ShooterGame3

Developed with Unreal Engine 4

Shooter Game sample in UE4 modified to implement various gameplay functionalities.
All functionalities have been implemented in C++ using Character Movement Component and have been tested to work in a Multiplayer Environment.
Testing has been done using packet lag of 500ms with a variance of 30%.

1. Teleportation - Teleports player 10 meters forward upon pressing T key. 
![Teleportation](https://user-images.githubusercontent.com/24374437/227433842-370dce4e-c92b-4ffd-83fe-75260c91a45a.png)

2. Jetpack - Players use energy fuel to fly using jetpack. Jetpack fuel is recharged when Jetpack is not in use. Fuel is displayed on player HUD. Press J to use Jetpack.
![Jetpack](https://user-images.githubusercontent.com/24374437/227434505-cc4aef74-916c-4ff6-aece-c34e611ff44c.png)

3. Wall Jump - Player is able to perform a lateral jump while flying close to a wall. The jump pushed player in the direction opposite to wall and a bit higher than before. 
![WallJump](https://user-images.githubusercontent.com/24374437/227434677-ece96cb6-2785-4500-b98a-df1b1bb926b8.png)

4. Freezing Gun - When hitting an opponent with this weapon it gets freezed and cannot move or shoot for a specific time. Use F to activate freeze gun.
![Freeze Gun](https://user-images.githubusercontent.com/24374437/227434942-0ae0ba98-3172-4554-b5a1-e6100f22428a.png)

5. Shrink Gun - When hitting an opponent with this weapon it gets shrinked for a specific time. Id during this time another player stomps on him, then he dies. The shrinking and eventually, unshrinking has been interpolated. Amount of shrink time remaining is displayed on player HUD using Shrink Bar.
![Shrink Gun](https://user-images.githubusercontent.com/24374437/227435316-1b484e71-c32e-47bb-b151-c3edf14838bb.png)

6. Time Rewind - Player is able to rewind time back for a specific time. Rewinding can be made faster or slower as per game requirements. Functionality is robust for high or low FPS. Charging of rewind time and remaining rewind time is displayed on player HUD using Rewind Bar. Press P to activate Time Rewind.
![Rewind](https://user-images.githubusercontent.com/24374437/227435716-ccce50f1-1c4d-4a56-a1a1-a34fb7e1e39c.png)

7. Wall Run - When player runs and then jumps towards a wall, then he starts running on Wall. Player can jump out of the wall at any moment.
![Wall Run](https://user-images.githubusercontent.com/24374437/227435901-2f6aa432-c3b4-4cf4-bc94-036b93353ce5.png)

8. Drop Weapon - Player drops weapon on being killed with current ammunition remaining. The pickup is inside the game for certain amount of time and then disappears.
![Drop Weapon](https://user-images.githubusercontent.com/24374437/227436042-62b91095-f5de-494b-96d0-2473e53717de.png)

Note - Blueprint Settings
![BP Settings](https://user-images.githubusercontent.com/24374437/227436083-82674117-8492-4431-b4b1-a1a6e4e651c4.png)

Testing & Other Settings
![Testing   Settings](https://user-images.githubusercontent.com/24374437/227436128-17789b66-c1c7-48fc-9003-8a45003aff76.png)
