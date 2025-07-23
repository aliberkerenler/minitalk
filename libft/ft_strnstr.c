/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aerenler <aerenler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/11 17:00:54 by aerenler          #+#    #+#             */
/*   Updated: 2024/11/14 18:25:50 by aerenler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *haystack, const char *needle, size_t len)
{
	const char	*str;
	const char	*subst;
	size_t		temp_len;

	if (*needle == '\0')
		return ((char *)haystack);
	while (*haystack != '\0' && len > 0)
	{
		str = haystack;
		subst = needle;
		temp_len = len;
		while (*subst != '\0' && *str == *subst && temp_len > 0)
		{
			str++;
			subst++;
			temp_len--;
		}
		if (*subst == '\0')
		{
			return ((char *)haystack);
		}
		haystack++;
		len--;
	}
	return (NULL);
}
