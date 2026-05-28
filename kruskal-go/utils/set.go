package utils

type Set struct {
	m map[interface{}]bool
}

func NewSet() *Set {
	s := &Set{}
	s.m = make(map[interface{}]bool)
	return s
}

func (s *Set) Add(value string) {
	s.m[value] = true
}

func (s *Set) Remove(value string) {
	delete(s.m, value)
}

func (s *Set) Contains(value string) bool {
	_, c := s.m[value]
	return c
}

func (s *Set) Keys() []string {
	keys := make([]string, 0)
	for k := range s.m {
		keys = append(keys, k.(string))
	}
	return keys
}
