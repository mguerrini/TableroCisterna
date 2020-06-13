package com.mercadolibre.cache;

import com.mercadolibre.utils.Describe;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Created by ppileci on 4/7/17.
 */
@RunWith(PowerMockRunner.class)
public class CacheTest {
    private static final CacheWrapper<Map> cache = new CacheWrapper<>(200);

    @Before
    public void setup(){
        cache.removeAll();
    }

    @Test
    @Describe(description = "Get should return null if element is not present")
    public void testGetForNotPresentElement(){
        assertThat(cache.get("not_found_agency")).isNull();
    }

    @Test
    @Describe(description = "Get should return agency if element is present in cache")
    public void testGetForExistentElement(){
        cache.add("test_agency", Collections.singletonMap("id","test_agency"));
        assertThat(cache.get("test_agency")).containsEntry("id","test_agency");
    }

    @Test
    @Describe(description = "Remove all should invalidate all cache keys")
    public void testRemoveAllCacheKeys(){
        cache.add("test_agency_1", Collections.singletonMap("id","test_agency_1"));
        cache.add("test_agency_2", Collections.singletonMap("id","test_agency_2"));
        cache.add("test_agency_3", Collections.singletonMap("id","test_agency_3"));
        assertThat(cache.get("test_agency_1")).isNotNull();
        assertThat(cache.get("test_agency_2")).isNotNull();
        assertThat(cache.get("test_agency_3")).isNotNull();

        cache.removeAll();

        assertThat(cache.get("test_agency_1")).isNull();
        assertThat(cache.get("test_agency_2")).isNull();
        assertThat(cache.get("test_agency_3")).isNull();
    }

    @Test
    @Describe(description = "Add all should add several agencies at the same time")
    public void testAddSeveralAgenciesAtTheSameTime(){
        Map agencies = new HashMap(){{
           put ("agency_1", Collections.singletonMap("name","Oca 1"));
           put ("agency_2", Collections.singletonMap("name","Oca 2"));
           put ("agency_3", Collections.singletonMap("name","Oca 3"));
        }};
        cache.addAll(agencies);

        assertThat(cache.get("agency_1")).containsEntry("name","Oca 1");
        assertThat(cache.get("agency_2")).containsEntry("name","Oca 2");
        assertThat(cache.get("agency_3")).containsEntry("name","Oca 3");
    }
}
