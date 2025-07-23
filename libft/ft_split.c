/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aerenler <aerenler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/20 14:08:46 by aerenler          #+#    #+#             */
/*   Updated: 2024/11/14 19:06:24 by aerenler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	count_words(char const *s, char c)
{
	int	count;
	int	in_word;

	count = 0;
	in_word = 0;
	while (*s)
	{
		if (*s != c && !in_word)
		{
			in_word = 1;
			count++;
		}
		else if (*s == c)
			in_word = 0;
		s++;
	}
	return (count);
}

static char	*get_next_word(char const *s, char c, int *start)
{
	int		end;
	int		i;
	char	*word;

	i = 0;
	end = *start;
	while (s[end] && s[end] != c)
		end++;
	word = (char *)malloc(end - *start + 1);
	if (!word)
		return (NULL);
	while (i < end - *start)
	{
		word[i] = s[*start + i];
		i++;
	}
	word[end - *start] = '\0';
	*start = end + 1;
	return (word);
}

void	*free_result(char **result, int index)
{
	if (!result[index])
	{
		while (index >= 0)
		{
			free(result[index]);
			index--;
		}
		free(result);
		return (NULL);
	}
	return (result);
}

char	**ft_split(char const *s, char c)
{
	int		word_count;
	char	**result;
	int		start;
	int		index;

	start = 0;
	index = 0;
	if (!s)
		return (NULL);
	word_count = count_words(s, c);
	result = malloc(sizeof(char *) * (word_count + 1));
	if (!result)
		return (NULL);
	while (index < word_count)
	{
		while (s[start] && s[start] == c)
			start++;
		result[index] = get_next_word(s, c, &start);
		if (!result[index])
			result = free_result(result, index);
		index++;
	}
	result[index] = NULL;
	return (result);
}
