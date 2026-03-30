
/**
 * A dirty simple hash function I quickly made
 */
unsigned short hash(char *line, int lineLength)
{
    unsigned short hash = 0;

    for (int i = 0; i < lineLength; i++)
    {
        hash = (line[i] - hash * i);
    }

    return hash;
}