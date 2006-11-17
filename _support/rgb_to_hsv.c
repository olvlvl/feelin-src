/*

   RGB to HSV using integers by Olivier LAVIALE & Hombre

   [Total number of possible value(360)]/6 = 60 ; change all "60" values by new
   ones if you want more precision ; AdjustRGB/object.c must be changed accordingly :
   hue slider max value = [Total number of possible value]-1

*/

#include <libraries/feelin.h>

void rgb_to_hsv(uint32 r,uint32 g,uint32 b,FHSV *hsv)
{
    uint32 max, min, delta;

    max = MAX(r, MAX(g, b));
    min = MIN(r, MIN(g, b));
    delta = max-min;

    if (max == 0)           /* Tous les curseurs RVB sont à 0 */
    {
        /* HOMBRE @ GOFROMIEL :

           IMPORTANT : Lorsque les trois curseurs sont à 0 lorsqu'on entre dans
                       cette fonction, la teinte est "indéterminée". La routine
                       appelante doit elle même initialiser  la structure HSV à
                       0,0,0 si c'est le cas,  ou de  préférence à  la dernière
                       teinte connu.
        */
        //hsv->h = 0;
        hsv->s = 255;
        hsv->v = 0;
        return;
    }
    else if (delta == 0)     /* Nuance de gris */
    {
        //hsv->h = 0;     Voir ci-dessus
        hsv->s = 0;
        hsv->v = max;  /* valeur = niveau de gris */
        return;
    }
    else
    {
        uint8 mileStone=0xFF, interval=0xFF, i;
        uint32 secondCursor=0;

        hsv->v = max;
        hsv->s = delta * 255 / max;

        if (r == max)
        {
            mileStone &= (uint8)(1<<0 | 1<<1 | 1<<5);
            interval  &= (uint8)(1<<0 | 1<<5);
        }
        else if (r == min)
        {
            mileStone &= (uint8)(1<<2 | 1<<3 | 1<<4);
            interval  &= (uint8)(1<<2 | 1<<3);
        }
        else
        {
            secondCursor = (r-min) * (255*1000) / delta;
            interval &= (uint8)(1<<1 | 1<<4);
        }

        if (g == max)
        {
            mileStone &= (uint8)(1<<1 | 1<<2 | 1<<3);
            interval  &= (uint8)(1<<1 | 1<<2);
        }
        else if (g == min)
        {
            mileStone &= (uint8)(1<<0 | 1<<4 | 1<<5);
            interval  &= (uint8)(1<<4 | 1<<5);
        }
        else
        {
            secondCursor = (g-min) * (255*1000) / delta;
            interval &= (uint8)(1<<0 | 1<<3);
        }

        if (b == max)
        {
            mileStone &= (uint8)(1<<3 | 1<<4 | 1<<5);
            interval  &= (uint8)(1<<3 | 1<<4);
        }
        else if (b == min)
        {
            mileStone &= (uint8)(1<<0 | 1<<1 | 1<<2);
            interval  &= (uint8)(1<<0 | 1<<1);
        }
        else
        {
            secondCursor = (b-min) * (255*1000) / delta;
            interval &= (uint8)(1<<2 | 1<<5);
        }

        /* On retrouve l'interval */
        if (interval)
        {
            for (i=0; i<6; i++)
            {
                if ((interval >> i) & (uint8)1)
                {
                    hsv->h = 60 * i +
                          (i%2 ?
                                 60 - secondCursor * 60 / (255*1000)
                               :      secondCursor * 60 / (255*1000)
                          );

 
                    return ;
                }
            }
        }
        /* Si pas d'interval trouvé, c'est qu'on est sur un des mileStones */
        else
        {
            for (i = 0 ; i < 7 ; i++)
            {
                if ((mileStone >> i) & (uint8)1)
                {
                    hsv->h = 60 * i;
                    return ;
                }
            }
        }

    /* Cela peut-il arriver là ? Dans le doute... */
    hsv->h = 0;
    return;

    }
}

