
/* field breakup for ircd.conf file. */
char *getfield(char *newline)
{
    static char *line = (char *) NULL;
    char       *end, *field;

    if (newline)
	line = newline;
    
    if (line == (char *) NULL)
	return ((char *) NULL);
    
    field = line;
    if ((end = strchr(line, IRCDCONF_DELIMITER)) == NULL)
    {
	line = (char *) NULL;
	if ((end = strchr(field, '\n')) == (char *) NULL)
	    end = field + strlen(field);
    }
    else
	line = end + 1;
    *end = '\0';
    return (field);
}
