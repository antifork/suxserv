
/* field breakup for ircd.conf file. */
gchar *getfield(gchar *newline)
{
    static gchar *line = (gchar *) NULL;
    gchar       *end, *field;

    if (newline)
	line = newline;
    
    if (line == (gchar *) NULL)
	return ((gchar *) NULL);
    
    field = line;
    if ((end = strchr(line, IRCDCONF_DELIMITER)) == NULL)
    {
	line = (gchar *) NULL;
	if ((end = strchr(field, '\n')) == (gchar *) NULL)
	    end = field + strlen(field);
    }
    else
	line = end + 1;
    *end = '\0';
    return (field);
}
