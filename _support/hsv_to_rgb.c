/*

   HSV to RGB using integers by Olivier LAVIALE & Hombre

   [Total number of possible value(360)]/6 = 60 ; change all "60" values by new
   ones if you want more precision ; AdjustRGB/object.c must be changed accordingly :
   hue slider max value = [Total number of possible value]-1
 
*/

#include <libraries/feelin.h>

///hsv_to_rgb
void hsv_to_rgb(unsigned int h,unsigned int s,unsigned int v,FRGB *rgb)
{
    uint32 red, green, blue;
    static FRGB mileStones[7] = {
        {255,0  ,0  },
        {255,255,0  },
        {0  ,255,0  },
        {0  ,255,255},
        {0  ,0  ,255},
        {255,0  ,255},
        {255,0  ,0  }
    };

    if (s == 0)
    {
        red = green = blue = (uint8)v*60000;
    }
    else
    {
        uint32 secondCursor;
        uint16 mileStone = h/60;

        /*
            Les calculs sont fait avec des entiers, mais multipliés par 60000 pour garder
            la précision (équivalent à 4 chiffres derrière la virgule) ; garantie sans
            dépassement si on garde 60 et 255 ! ;-)

            Memo : coef < (uint32.max / 255 / MAX((60-1,255) ; si 60 (360/6) ou 255 qui
            sont les valeurs maxi des sliders sont changé par autre chose, recalculer un
            nouveau coeff...
         */
        red   = (uint32)mileStones[mileStone].r*60000;
        green = (uint32)mileStones[mileStone].g*60000;
        blue  = (uint32)mileStones[mileStone].b*60000;

        secondCursor = ((uint32)(h % 60)) * (255*60000) / 60;

        switch (mileStone)
        {
        case 1 :
            red = 255*60000 - secondCursor;
            break;
        case 4 :
            red = secondCursor;
            break;
        case 0 :
            green = secondCursor;
            break;
        case 3 :
            green = 255*60000 - secondCursor;
            break;
        case 2 :
            blue = secondCursor;
            break;
        case 5 :
            blue = 255*60000 - secondCursor;
            break;
        }

        if (s != 255)
        {
            /* La saturation n'est pas maximum, donc on applatis les curseurs vers 255 */
            if (red   < 255*60000) red   = 255*60000 - (255*60000 - red  ) * s / 255 ;
            if (green < 255*60000) green = 255*60000 - (255*60000 - green) * s / 255 ;
            if (blue  < 255*60000) blue  = 255*60000 - (255*60000 - blue ) * s / 255 ;
        }
        if (v != 255)
        {
            /* La brillance n'est pas maximum, donc on ramène les curseurs vers 0 */
            if (red  ) red   = red   * v / 255;
            if (green) green = green * v / 255;
            if (blue ) blue  = blue  * v / 255;
        }
    }

    rgb->r = (uint8)(red  /60000);
    rgb->g = (uint8)(green/60000);
    rgb->b = (uint8)(blue /60000);

    return ;
}
//+

